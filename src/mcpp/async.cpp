#include <mcpp/async.hpp>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <limits>
#include <thread>
#include <utility>

namespace mcpp {

constexpr std::uint64_t ref = std::uint64_t(1) << 32U;
constexpr std::uint64_t running = 1;
constexpr std::uint64_t max = std::numeric_limits<std::uint64_t>::max();
constexpr std::uint64_t stop_flag = max & ~(max >> 1U);

static bool is_last (std::uint64_t val) noexcept {
	constexpr std::uint64_t unshifted = std::numeric_limits<std::uint32_t>::max() >> 1U;
	constexpr std::uint64_t mask = unshifted << 32U;
	return (val & mask) == ref;
}

static bool is_stopped (std::uint64_t val) noexcept {
	return bool(val & stop_flag);
}

static std::uint64_t running_count (std::uint64_t val) noexcept {
	return val & std::numeric_limits<std::uint32_t>::max();
}

void async::pointer::destroy () noexcept {
	if (!ptr_) return;
	auto val = ptr_->fetch_sub(ref, std::memory_order_acq_rel);
	if (is_last(val)) {
		assert(is_stopped(val));
		delete ptr_;
	}
	ptr_ = nullptr;
}

void async::pointer::acquire () noexcept {
	assert(ptr_);
	ptr_->fetch_add(ref, std::memory_order_acq_rel);
}

async::pointer::pointer () : ptr_(new std::atomic<std::uint64_t>(ref)) {	}

async::pointer::pointer (const pointer & other) noexcept : ptr_(other.ptr_) {
	acquire();
}

async::pointer::pointer (pointer && other) noexcept : ptr_(other.ptr_) {
	other.ptr_ = nullptr;
}

async::pointer & async::pointer::operator = (const pointer & rhs) noexcept {
	if (this != &rhs) {
		destroy();
		ptr_ = rhs.ptr_;
		acquire();
	}
	return *this;
}

async::pointer & async::pointer::operator = (pointer && rhs) noexcept {
	destroy();
	using std::swap;
	swap(ptr_, rhs.ptr_);
	return *this;
}

async::pointer::~pointer () noexcept {
	destroy();
}

void async::pointer::complete () noexcept {
	assert(ptr_);
	ptr_->fetch_sub(running, std::memory_order_acq_rel);
}

bool async::pointer::invoke () noexcept {
	assert(ptr_);
	auto val = ptr_->fetch_add(running, std::memory_order_acq_rel);
	if (!is_stopped(val)) return true;
	complete();
	return false;
}

void async::pointer::stop () noexcept {
	assert(ptr_);
	auto val = ptr_->fetch_or(stop_flag, std::memory_order_acq_rel);
	while (running_count(val) != 0) val = ptr_->load(std::memory_order_acquire);
}

bool async::pointer::stopped () const noexcept {
	assert(ptr_);
	return is_stopped(ptr_->load(std::memory_order_acquire));
}

async::~async () noexcept {
	assert(ptr_.stopped());
}

void async::stop () noexcept {
	ptr_.stop();
}

}

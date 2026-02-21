#include "wakeup/gate.hpp"

#include <spdlog/spdlog.h>

namespace xiaoai_plus::wakeup {

Gate::Gate(std::chrono::milliseconds timeout, Hooks hooks)
    : timeout_(timeout.count() > 0 ? timeout : std::chrono::seconds(15)), hooks_(std::move(hooks)) {
  timeout_thread_ = std::thread([this]() { TimeoutLoop(); });
}

Gate::~Gate() { Close(); }

Step Gate::step() const {
  std::lock_guard<std::mutex> lock(mu_);
  return step_;
}

bool Gate::TryWakeup(const std::string& keyword) {
  const std::string reason = "local_kws: " + keyword;
  std::function<void(const std::string&)> on_arm;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (step_ != Step::kIdle) {
      spdlog::info("gate reject wakeup: busy");
      return false;
    }
    step_ = Step::kActive;
    RefreshTimeoutLocked();
    on_arm = hooks_.on_arm;
  }
  spdlog::info("gate to active: {}", reason);
  if (on_arm) {
    on_arm(reason);
  }
  return true;
}

void Gate::Activate(const std::string& reason) {
  bool was_idle = false;
  std::function<void(const std::string&)> on_arm;
  {
    std::lock_guard<std::mutex> lock(mu_);
    was_idle = (step_ == Step::kIdle);
    step_ = Step::kActive;
    RefreshTimeoutLocked();
    on_arm = hooks_.on_arm;
  }
  if (was_idle && on_arm) {
    on_arm(reason);
  }
}

void Gate::Disarm(const std::string& reason) {
  std::function<void(const std::string&)> after;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (step_ == Step::kIdle) {
      return;
    }
    step_ = Step::kIdle;
    ++timeout_epoch_;
    after = hooks_.after_disarm;
  }
  spdlog::info("gate to idle: {}", reason);
  timeout_cv_.notify_all();
  if (after) {
    after(reason);
  }
}

void Gate::RefreshTimeout() {
  std::lock_guard<std::mutex> lock(mu_);
  if (step_ == Step::kActive) {
    RefreshTimeoutLocked();
  }
}

void Gate::RefreshTimeoutLocked() {
  timeout_deadline_ = std::chrono::steady_clock::now() + timeout_;
  ++timeout_epoch_;
  timeout_cv_.notify_all();
}

void Gate::SetAiSpeaking(bool is_speaking) {
  std::lock_guard<std::mutex> lock(mu_);
  is_ai_speaking_ = is_speaking;
  if (is_ai_speaking_ && step_ == Step::kActive) {
    RefreshTimeoutLocked();
  }
}

void Gate::Close() {
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (closed_) {
      return;
    }
    closed_ = true;
    ++timeout_epoch_;
  }
  timeout_cv_.notify_all();
  if (timeout_thread_.joinable()) {
    timeout_thread_.join();
  }
}

void Gate::TimeoutLoop() {
  std::unique_lock<std::mutex> lock(mu_);
  while (!closed_) {
    if (step_ != Step::kActive) {
      timeout_cv_.wait(lock, [this]() { return closed_ || step_ == Step::kActive; });
      continue;
    }

    const auto epoch = timeout_epoch_;
    const auto deadline = timeout_deadline_;
    if (timeout_cv_.wait_until(lock, deadline, [this, epoch]() {
          return closed_ || step_ != Step::kActive || timeout_epoch_ != epoch;
        })) {
      continue;
    }

    step_ = Step::kIdle;
    ++timeout_epoch_;
    auto after = hooks_.after_disarm;
    lock.unlock();
    spdlog::info("gate timeout -> idle");
    if (after) {
      after("timeout");
    }
    lock.lock();
  }
}

}  // namespace xiaoai_plus::wakeup

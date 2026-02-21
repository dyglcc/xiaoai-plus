#include "audio/player.hpp"

#include <chrono>
#include <cstdio>
#include <sstream>
#include <system_error>

#include <spdlog/spdlog.h>

namespace xiaoai_plus::audio {

AplayPlayer::AplayPlayer(config::Audio cfg, int queue_frames)
    : cfg_(std::move(cfg)), queue_frames_(queue_frames > 0 ? queue_frames : 128) {}

AplayPlayer::~AplayPlayer() { Close(); }

bool AplayPlayer::Start() {
  if (running_.exchange(true)) {
    return true;
  }

  try {
    thread_ = std::thread([this]() { WriteLoop(); });
  } catch (const std::system_error& e) {
    spdlog::error("player thread creation failed: {}", e.what());
    running_.store(false);
    return false;
  }
  return true;
}

bool AplayPlayer::Play(const std::vector<uint8_t>& chunk) {
  if (chunk.empty()) {
    return true;
  }

  std::lock_guard<std::mutex> lock(mu_);
  if (static_cast<int>(queue_.size()) >= queue_frames_) {
    return false;
  }
  queue_.push_back(chunk);
  cv_.notify_one();
  return true;
}

void AplayPlayer::Interrupt() {
  {
    std::lock_guard<std::mutex> lock(mu_);
    queue_.clear();
  }
  std::lock_guard<std::mutex> pipe_lock(pipe_mu_);
  ClosePipeLocked();
}

void AplayPlayer::SetOnChunkPlayed(std::function<void(const std::vector<uint8_t>&)> cb) {
  std::lock_guard<std::mutex> lock(cb_mu_);
  on_chunk_played_ = std::move(cb);
}

bool AplayPlayer::OpenPipeLocked() {
  if (pipe_) {
    return true;
  }

  std::ostringstream cmd;
  cmd << "aplay --quiet -t raw"
      << " -D " << cfg_.output_device
      << " -f S" << cfg_.bits_per_sample << "_LE"
      << " -r " << cfg_.sample_rate
      << " -c " << cfg_.channels
      << " --buffer-size " << cfg_.buffer_size
      << " --period-size " << cfg_.period_size
      << " -";

  pipe_ = popen(cmd.str().c_str(), "w");
  if (!pipe_) {
    spdlog::warn("player popen aplay failed");
    return false;
  }
  return true;
}

void AplayPlayer::ClosePipeLocked() {
  if (pipe_) {
    pclose(pipe_);
    pipe_ = nullptr;
  }
}

void AplayPlayer::WriteLoop() {
  const auto idle_timeout = std::chrono::milliseconds(1200);
  while (running_.load()) {
    std::vector<uint8_t> chunk;
    bool timed_out = false;
    {
      std::unique_lock<std::mutex> lock(mu_);
      if (!cv_.wait_for(lock, idle_timeout, [this]() { return !running_.load() || !queue_.empty(); })) {
        timed_out = true;
      }
      if (!running_.load()) {
        break;
      }
      if (timed_out && queue_.empty()) {
        lock.unlock();
        std::lock_guard<std::mutex> pipe_lock(pipe_mu_);
        ClosePipeLocked();
        continue;
      }
      chunk = std::move(queue_.front());
      queue_.pop_front();
    }

    if (chunk.empty()) {
      continue;
    }

    bool write_ok = false;
    {
      std::lock_guard<std::mutex> pipe_lock(pipe_mu_);
      if (!OpenPipeLocked()) {
        continue;
      }
      size_t written = fwrite(chunk.data(), 1, chunk.size(), pipe_);
      if (written < chunk.size() || fflush(pipe_) != 0) {
        spdlog::warn("player write failed (written={}/{})", written, chunk.size());
        ClosePipeLocked();
      } else {
        write_ok = true;
      }
    }
    if (!write_ok) {
      continue;
    }

    std::function<void(const std::vector<uint8_t>&)> cb;
    {
      std::lock_guard<std::mutex> lock(cb_mu_);
      cb = on_chunk_played_;
    }
    if (cb) {
      cb(chunk);
    }
  }

}

void AplayPlayer::Close() {
  if (!running_.exchange(false)) {
    return;
  }

  cv_.notify_all();

  if (thread_.joinable()) {
    thread_.join();
  }

  {
    std::lock_guard<std::mutex> pipe_lock(pipe_mu_);
    ClosePipeLocked();
  }

  std::lock_guard<std::mutex> lock(mu_);
  queue_.clear();
}

}  // namespace xiaoai_plus::audio

#pragma once
#include <functional>
#include <string>
#include <string_view>

namespace room {
inline constexpr std::size_t max_line = 1024;

// Streams carry bytes, not messages. Retain partial lines between reads.
class LineFramer {
  public:
    using LineHandler = std::function<void(const std::string&)>;
    using OverflowHandler = std::function<void()>;

    void reset();
    void feed(
        std::string_view data, const LineHandler& on_line,
        const OverflowHandler& on_overflow = [] {});

  private:
    std::string buffer_;
    bool discarding_{false};
};
} // namespace room

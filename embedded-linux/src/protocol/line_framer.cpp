#include "line_framer.hpp"

namespace room {
void LineFramer::reset() {
    buffer_.clear();
    discarding_ = false;
}

void LineFramer::feed(std::string_view data, const LineHandler& on_line,
                      const OverflowHandler& on_overflow) {
    for (char character : data) {
        if (character == '\n') {
            if (!discarding_) {
                if (!buffer_.empty() && buffer_.back() == '\r') {
                    buffer_.pop_back();
                }
                if (!buffer_.empty()) {
                    on_line(buffer_);
                }
            }
            reset();
        } else if (!discarding_) {
            if (buffer_.size() >= max_line) {
                // Discard the whole frame; its suffix must never become a command.
                buffer_.clear();
                discarding_ = true;
                on_overflow();
            } else {
                buffer_ += character;
            }
        }
    }
}
} // namespace room

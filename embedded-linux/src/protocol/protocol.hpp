#pragma once
#include "line_framer.hpp"
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace room {
struct State {
    bool occupied{};
    bool light_on{};
    bool contact_open{};
    std::string alarm{"none"};
};

struct Message {
    std::string kind;
    std::map<std::string, std::string> fields;
    std::optional<State> state;
};

// Invalid or incomplete messages never become trusted device state.
std::optional<Message> parse(std::string_view line);
std::string format(const State& state);
} // namespace room

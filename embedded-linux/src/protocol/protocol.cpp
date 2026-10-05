#include "protocol.hpp"
#include <sstream>
#include <stdexcept>

namespace room {
std::optional<Message> parse(std::string_view line) {
    if (line.size() > max_line)
        return {};
    std::istringstream input{std::string(line)};
    std::string prefix, token;
    Message m;
    if (!(input >> prefix >> m.kind) || prefix != "ROOM")
        return {};
    while (input >> token) {
        auto split = token.find('=');
        if (split == std::string::npos || split == 0 || split + 1 == token.size())
            return {};
        if (!m.fields.emplace(token.substr(0, split), token.substr(split + 1)).second)
            return {};
    }
    auto boolean = [&](const char* key) -> bool {
        auto value = m.fields.at(key);
        if (value != "0" && value != "1")
            throw std::invalid_argument("boolean");
        return value == "1";
    };
    try {
        if (m.kind == "STATE" || m.kind == "EVENT") {
            State s{boolean("occupied"), boolean("light_on"), boolean("contact_open"),
                    m.fields.at("alarm")};
            if (s.alarm != "none" && s.alarm != "warning" && s.alarm != "alarm")
                return {};
            if (m.kind == "EVENT") {
                if (m.fields.at("type").empty())
                    return {};
                const auto& mask = m.fields.at("changed");
                if (mask.size() != 10 || mask.substr(0, 2) != "0x" ||
                    mask.find_first_not_of("0123456789abcdefABCDEF", 2) != std::string::npos)
                    return {};
            }
            m.state = s;
        } else if (m.kind == "ACK") {
            const auto& p = m.fields.at("property");
            if (p != "occupied" && p != "light_on" && p != "contact_open" && p != "alarm")
                return {};
            (void)boolean("changed");
        } else if (m.kind == "ERR") {
            if (m.fields.at("code").empty())
                return {};
        } else if (m.kind == "BOOT") {
            if (m.fields.at("proto") != "1" || m.fields.at("node").empty())
                return {};
        } else if (m.kind == "PONG") {
            if (!m.fields.empty())
                return {};
        } else
            return {};
    } catch (const std::exception&) {
        return {};
    }
    return m;
}

std::string format(const State& s) {
    return "ROOM STATE occupied=" + std::to_string(s.occupied) +
           " light_on=" + std::to_string(s.light_on) +
           " contact_open=" + std::to_string(s.contact_open) + " alarm=" + s.alarm;
}
} // namespace room

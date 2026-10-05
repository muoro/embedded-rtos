#include "gateway/controller.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x)) {                                                                                \
            std::cerr << "Failed line " << __LINE__ << ": " #x "\n";                               \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (false)
int main() {
    using namespace room;
    std::vector<std::string> lines;
    LineFramer f;
    int overflow = 0;
    auto collect = [&](const std::string& s) { lines.push_back(s); };
    f.feed("ROOM PO", collect);
    CHECK(lines.empty());
    f.feed("NG\r\nROOM PONG\n\n", collect);
    CHECK(lines.size() == 2 && lines[0] == "ROOM PONG");
    f.feed(std::string(max_line + 20, 'x') + "SET light_on 1\nROOM PONG\n", collect,
           [&] { ++overflow; });
    CHECK(overflow == 1 && lines.size() == 3 && lines.back() == "ROOM PONG");
    CHECK(parse("ROOM PONG"));
    CHECK(parse("ROOM BOOT proto=1 node=smart_room"));
    CHECK(parse("ROOM EVENT type=button changed=0x00000001 occupied=1 light_on=1 contact_open=0 "
                "alarm=none"));
    const auto off = "ROOM STATE occupied=0 light_on=0 contact_open=0 alarm=none";
    const auto on = "ROOM STATE occupied=0 light_on=1 contact_open=0 alarm=none";
    CHECK(parse(off));
    for (auto bad :
         {"ROOM STATE occupied=2 light_on=0 contact_open=0 alarm=none",
          "ROOM STATE occupied=0 light_on=0 contact_open=0 alarm=oops",
          "ROOM STATE occupied=0 light_on=0 alarm=none", "ROOM ACK property=light_on changed=x",
          "ROOM PONG x=1", "ROOM ACK property=light_on property=alarm changed=1", "garbage",
          "ROOM BOOT proto=2 node=x"})
        CHECK(!parse(bad));
    std::vector<std::string> tx, ui;
    Controller c([&](const std::string& s) { tx.push_back(s); },
                 [&](const std::string& s) { ui.push_back(s); });
    auto has = [&](const std::string& fragment) {
        for (auto& s : ui)
            if (s.find(fragment) != std::string::npos)
                return true;
        return false;
    };
    c.start(0);
    CHECK(tx.size() == 2 && tx[1] == "GET STATE");
    c.command("SET light_on 1", 1);
    CHECK(has("status=offline"));
    c.receive(off, 10);
    CHECK(c.status_line().find("ready=1") != std::string::npos);
    c.command("SET light_on 1", 20);
    CHECK(tx.back() == "SET light_on 1");
    CHECK(has("status=pending"));
    c.receive(on, 25);
    CHECK(!has("status=confirmed"));
    c.command("SET light_on 0", 26);
    CHECK(has("status=busy"));
    c.receive("ROOM ACK property=light_on changed=1", 30);
    CHECK(tx.back() == "GET STATE");
    c.receive(on, 40);
    CHECK(has("status=confirmed"));
    c.command("SET light_on 0", 50);
    c.receive("ROOM ACK property=light_on changed=0", 60);
    c.receive(on, 70);
    CHECK(has("status=rejected"));
    ui.clear();
    tx.clear();
    c.command("SET light_on 0", 80);
    c.tick(3080);
    CHECK(has("status=timeout") && tx.back() == "GET STATE");
    int sets = 0;
    for (auto& s : tx)
        if (s == "SET light_on 0")
            ++sets;
    CHECK(sets == 1);
    c.receive(off, 3100);
    CHECK(c.status_line().find("ready=0") != std::string::npos);
    c.receive("ROOM PONG", 6100);
    CHECK(c.status_line().find("ready=1") != std::string::npos);
    c.command("SET light_on 1", 6110);
    c.receive("ROOM BOOT proto=1 node=smart_room", 6120);
    CHECK(has("reason=device_reset") && c.status_line().find("valid=0") != std::string::npos);
    c.receive(off, 6130);
    c.tick(12130);
    CHECK(c.status_line().find("online=0 valid=0") != std::string::npos);
    c.receive("not a valid message", 12140);
    CHECK(c.status_line().find("online=0") != std::string::npos);
    std::cout << "Framing, parser and controller checks passed\n";
}

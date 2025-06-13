#include "core.h"
#include "net.h"
#include "log.h"
#include "timer.h"
using namespace coruring::async;
using namespace coruring::socket::net;
using namespace coruring::log;
using namespace coruring::timer;
using namespace coruring::scheduler;

Scheduler sched{2};

Task<> process(TcpStream stream) {
    char buf[1024];
    while (true) {
        auto ok = co_await stream.read(buf);
        if (!ok) {
            console.error(ok.error().message());
            break;
        }
        if (ok.value() == 0) {
            console.info("Connection closed");
            break;
        }

        auto len = ok.value();
        buf[len] = '\0';
        console.info("read {}", buf);

        ok = co_await stream.write({buf, len});
        if (!ok) {
            console.error(ok.error().message());
            break;
        }
    }
}

Task<> server() {
    auto has_addr = SocketAddr::parse("127.0.0.1", 8080);
    if (!has_addr) {
        console.error(has_addr.error().message());
        co_return;
    }
    auto has_listener = TcpListener::bind(has_addr.value());
    if (!has_listener) {
        console.error(has_listener.error().message());
        co_return;
    }
    auto listener = std::move(has_listener.value());
    while (true) {
        auto has_stream = co_await listener.accept();
        if (!has_stream) {
            console.error(has_stream.error().message());
            break;
        }
        auto& [stream, peer_addr] = has_stream.value();
        console.info("Accepted connection {}", peer_addr);
        sched.spawn(process(std::move(stream)));
    }
}

int main() {
    sched.spawn(server());
    sched.run();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


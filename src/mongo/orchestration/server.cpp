#include "mongo/orchestration/server.h"

namespace mongo {
namespace orchestration {

    Server::Server(const string& url, Json::Value parameters) : Resource(url) {
    }

    void Server::start() {
        action("start");
    }

    void Server::stop() {
        action("stop");
    }

    void Server::restart() {
        action("restart");
    }

    void Server::destroy() {
        del();
    }

    RestClient::response Server::status() const {
        return get();
    }

    string Server::uri() const {
        return handle_response(status())["uri"].asString();
    }

} // namespace orchestration
} // namespace mongo

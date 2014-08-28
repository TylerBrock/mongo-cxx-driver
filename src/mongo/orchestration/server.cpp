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

    string Server::uri() const {
        Json::Value doc;
        Json::Reader reader;
        reader.parse(status().body.c_str(), doc);
        return doc["uri"].asString();
    }

} // namespace orchestration
} // namespace mongo

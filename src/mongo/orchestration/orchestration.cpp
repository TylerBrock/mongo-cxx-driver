/**
 * Copyright 2014 MongoDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>
#include <stdexcept>

#include "orchestration.h"

namespace mongo {
namespace orchestration {

    enum Status {
        OK = 200,
        NoContent = 204,
        BadRequest = 400,
        NotFound = 404,
        InternalServerError = 500
    };

    namespace URI {
        const char kServers[] = "servers";
        const char kReplicaSets[] = "replica_sets";
        const char kShardedCluster[] = "sharded_cluster";
    }

    const char Resource::_content_type[] = "text/json";

    Resource::Resource(string _url) : _url(_url) {}

    RestClient::response Resource::get(string relative_path) const {
        return RestClient::get(make_url(relative_path));
    }

    RestClient::response Resource::put(string relative_path, string payload) {
        return RestClient::put(make_url(relative_path), _content_type, payload);
    }

    RestClient::response Resource::post(string relative_path, string payload) {
        return RestClient::post(make_url(relative_path), _content_type, payload);
    }

    RestClient::response Resource::del(string relative_path) {
        return RestClient::del(make_url(relative_path));
    }

    RestClient::response Resource::action(string action) {
        Json::Value doc;
        Json::FastWriter writer;
        doc["action"] = action;
        return post("", writer.write(doc));
    }

    string Resource::make_url(string relative_path) const {
        return relative_path.empty() ? _url : _url + "/" + relative_path;
    }

    auto_ptr<Json::Value> Resource::handle_response(RestClient::response response) const {
        auto_ptr<Json::Value> doc_ptr(new Json::Value);
        if (response.code == OK) {
            Json::Reader reader;
            bool parseSuccessful = reader.parse(response.body.c_str(), *doc_ptr);
            if (!parseSuccessful)
                throw std::runtime_error("Failed to parse response: " + response.body);
        } else if (response.code != NoContent) {
            throw std::runtime_error("Failed got a bad response: " + response.body);
        }
        return doc_ptr;
    }

    Service::Service(string url) : Resource(url) {}

    vector<Server> Service::servers() const {
        return get_plural_resource<Server>("servers");
    }

    vector<ReplicaSet> Service::replica_sets() const {
        return get_plural_resource<ReplicaSet>("rs");
    }

    vector<Cluster> Service::clusters() const {
        return get_plural_resource<Cluster>("sh");
    }

    Server Service::server(const string& id) const {
        return Server(_url + "/servers/" + id);
    }

    ReplicaSet Service::replica_set(const string& id) const {
        return ReplicaSet(_url + "/replica_sets/" + id);
    }

    string Service::createMongod(const Json::Value& params) {
        Json::Value doc(params);

        doc["name"] = "mongod";
        doc["procParams"]["setParameter"]["enableTestCommands"] = 1;

        Json::FastWriter writer;

        RestClient::response result = post(URI::kServers, writer.write(doc));
        auto_ptr<Json::Value> result_doc = handle_response(result);
        return (*result_doc)["id"].asString();
    }

    string Service::createReplicaSet(const Json::Value& params) {
        auto_ptr<Json::Value> result_doc = handle_response(post(URI::kReplicaSets, "{\"members\": [{},{}]}"));
        return (*result_doc)["id"].asString();
    }

    Server::Server(const string& url) : Resource(url) {}

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
        assert(doc.isObject());
        assert(doc.isMember("uri"));
        return doc["uri"].asString();
    }

    ReplicaSet::ReplicaSet(const string& url) : Resource(url) {}

    string ReplicaSet::uri() const {
        Json::Value doc;
        Json::Reader reader;
        reader.parse(status().body.c_str(), doc);
        assert(doc.isObject());
        assert(doc.isMember("uri"));
        return string("mongodb://") + doc["uri"].asString();
    }

    Server ReplicaSet::primary() const {
        auto_ptr<Json::Value> doc = handle_response(get("primary"));
        string primary_uri = (*doc)["uri"].asString();
        return Server(_url.substr(0, _url.find("/")) + primary_uri);
    }

    vector<Server> ReplicaSet::secondaries() const {
        vector<Server> secondaries;
        auto_ptr<Json::Value> doc = handle_response(get("secondaries"));

        for (unsigned i=0; i<doc->size(); i++) {
            string secondary_uri = (*doc)[i]["uri"].asString();
            Server secondary(_url.substr(0, _url.find("/")) + secondary_uri);
            secondaries.push_back(secondary);
        }

        return secondaries;
    }

    RestClient::response ReplicaSet::status() const {
        return get();
    }

    RestClient::response Server::status() const {
        return get();
    }

    void ReplicaSet::destroy() {
        del();
    }

    Cluster::Cluster(const string& url) : Resource(url) {}

} // namespace orchestration
} // namespace mongo

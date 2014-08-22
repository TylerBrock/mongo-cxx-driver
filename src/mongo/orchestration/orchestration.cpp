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

#include "orchestration.h"

#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/reader.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"

namespace mongo {
namespace orchestration {

    namespace {
        const int kStatusOK = 200;
        const int kStatusFail = 400;
        const int kStatusNotFound = 404;
    }

    using namespace rapidjson;

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

    string Resource::make_url(string relative_path) const {
        return relative_path.empty() ? _url : _url + "/" + relative_path;
    }

    API::API(string url) : Resource(url) {}

    vector<Host> API::hosts() const {
        vector<Host> hosts;

        RestClient::response result = get("/hosts");
        Document hosts_doc;
        hosts_doc.Parse(result.body.c_str());
        // TODO: make hosts from info here
        return hosts;
    }

    vector<ReplicaSet> API::replica_sets() const {
        vector<ReplicaSet> replica_sets;
        RestClient::response result = get("/hosts");
        Document sets_doc;
        sets_doc.Parse(result.body.c_str());
        return replica_sets;
    }

    Host API::host(string id) const {
        return Host(_url + "/hosts/" + id);
    }

    ReplicaSet API::replica_set(string id) const {
        return ReplicaSet(_url + "/rs/" + id);
    }

    string API::createMongod(string id) {
        Document doc;
        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        writer.StartObject();

        // Host ID
        if (!id.empty()) {
            writer.String("id");
            writer.String(id.c_str());
        }

        // Process Name
        writer.String("name");
        writer.String("mongod");

        // Process Parameters
        writer.String("procParams");
        writer.StartObject();
        writer.String("setParameter");
        writer.String("enableTestCommands=1");
        writer.EndObject();

        writer.EndObject();

        RestClient::response result = post("hosts", sb.GetString());
        Document result_doc;
        result_doc.Parse(result.body.c_str());
        return result_doc["id"].GetString();
    }

    Host::Host(string url) : Resource(url) {}

    void Host::start() {
        RestClient::response response = put("start");
        //if (response.code == kStatusOK)
    }

    void Host::stop() {
        put("stop");
    }

    void Host::restart() {
        put("restart");
    }

    void Host::destroy() {
        del();
    }

    string Host::uri() const {
        Document doc;
        doc.Parse(status().body.c_str());
        return doc["uri"].GetString();
    }

    ReplicaSet::ReplicaSet(string url) : Resource(url) {}

    RestClient::response Host::status() const {
        return get();
    }

} // namespace orchestration
} // namespace mongo

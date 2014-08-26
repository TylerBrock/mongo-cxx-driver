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

#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/reader.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"

namespace mongo {
namespace orchestration {

    namespace Status {
        const int OK = 200;
        const int NoContent = 204;
        const int BadRequest = 400;
        const int NotFound = 404;
        const int InternalServerError = 500;
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

    string Resource::make_url(string relative_path) const {
        return relative_path.empty() ? _url : _url + "/" + relative_path;
    }

    auto_ptr<Document> Resource::handle_response(RestClient::response response) const {
        auto_ptr<Document> doc_ptr(new Document);
        if (response.code == Status::OK) {
            doc_ptr->Parse(response.body.c_str());
            if (doc_ptr->HasParseError())
                throw std::runtime_error("Failed to parse response: " + response.body);
        } else if (response.code != Status::NoContent) {
            throw std::runtime_error("Failed got a bad response: " + response.body);
        }
        return doc_ptr;
    }

    API::API(string url) : Resource(url) {}

    vector<Host> API::hosts() const {
        return get_plural_resource<Host>("hosts");
    }

    vector<ReplicaSet> API::replica_sets() const {
        return get_plural_resource<ReplicaSet>("rs");
    }

    vector<Cluster> API::clusters() const {
        return get_plural_resource<Cluster>("sh");
    }

    Host API::host(const string& id) const {
        return Host(_url + "/hosts/" + id);
    }

    ReplicaSet API::replica_set(const string& id) const {
        return ReplicaSet(_url + "/rs/" + id);
    }

    string API::createMongod(const Document& params) {
        Document doc;
        Document::AllocatorType& alloc = doc.GetAllocator();
        doc.SetObject();

        // Copy params over
        if (params.IsObject())
            doc.CopyFrom(params, alloc);

        // Process Name
        doc.AddMember("name", "mongod", alloc);

        Value set_params(kObjectType);
        set_params.AddMember("enableTestCommands", 1, alloc);

        Value proc_params(kObjectType);
        proc_params.AddMember("setParameter", set_params, alloc);

        // Process Parameters
        doc.AddMember("procParams", proc_params, alloc);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        doc.Accept(writer);

        RestClient::response result = post("hosts", sb.GetString());
        auto_ptr<Document> result_doc = handle_response(result);
        return (*result_doc)["id"].GetString();
    }

    string API::createReplicaSet(const Document& params) {
        auto_ptr<Document> result_doc = handle_response(post("rs", "{\"members\": [{},{},{}]}"));
        return (*result_doc)["id"].GetString();
    }

    Host::Host(const string& url) : Resource(url) {}

    void Host::start() {
        put("start");
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
        assert(doc.IsObject());
        assert(doc.HasMember("uri"));
        return doc["uri"].GetString();
    }

    ReplicaSet::ReplicaSet(const string& url) : Resource(url) {}

    string ReplicaSet::uri() const {
        Document doc;
        doc.Parse(status().body.c_str());
        assert(doc.IsObject());
        assert(doc.HasMember("uri"));
        return string("mongodb://") + doc["uri"].GetString();
    }

    RestClient::response ReplicaSet::status() const {
        return get();
    }

    RestClient::response Host::status() const {
        return get();
    }

    void ReplicaSet::destroy() {
        del();
    }

    Cluster::Cluster(const string& url) : Resource(url) {}

} // namespace orchestration
} // namespace mongo

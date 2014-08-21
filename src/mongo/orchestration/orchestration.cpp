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

    RestClient::response Resource::get(string relative_path) {
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

    string Resource::make_url(string relative_path) {
        return relative_path.empty() ? _url : _url + "/" + relative_path;
    }

    API::API(string url) : Resource(url) {}

    Hosts API::hosts() { return Hosts(_url + "/hosts"); }

    Hosts::Hosts(string url) : Resource(url) {}

    Host Hosts::create(string process_type) {
        Document doc;
        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        writer.StartObject();
        writer.String("name");
        writer.String(process_type.c_str());
        writer.EndObject();
        RestClient::response result = post("", sb.GetString());

        Document host_doc;
        host_doc.Parse(result.body.c_str());

        return Host(_url + "/" + host_doc["id"].GetString());
    }

    void Hosts::destroy(Host h) {
    }

    Host::Host(std::string url) : Resource(url) {}

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

    string Host::uri() {
        Document doc;
        doc.Parse(status().body.c_str());
        return doc["uri"].GetString();
    }

    RestClient::response Host::status() {
        return get();
    }

} // namespace orchestration
} // namespace mongo

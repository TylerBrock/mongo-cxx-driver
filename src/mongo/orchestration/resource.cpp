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

#include "mongo/orchestration/resource.h"

namespace mongo {
namespace orchestration {

    namespace {
        const char content_type[] = "text/json";
    }

    enum Status {
        OK = 200,
        NoContent = 204,
        BadRequest = 400,
        NotFound = 404,
        InternalServerError = 500
    };

    Resource::Resource(string url)
        : _url(url)
    {}

    RestClient::response Resource::get(string relative_path) const {
        return RestClient::get(make_url(relative_path));
    }

    RestClient::response Resource::put(string relative_path, string payload) {
        return RestClient::put(make_url(relative_path), content_type, payload);
    }

    RestClient::response Resource::post(string relative_path, string payload) {
        return RestClient::post(make_url(relative_path), content_type, payload);
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

} // namespace orchestration
} // namespace mongo

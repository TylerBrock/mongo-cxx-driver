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

    Resource::Resource(const string& url)
        : _url(url)
    {}

    Resource::~Resource() {

    }

    RestClient::response Resource::get(const string& relative_path) const {
        return RestClient::get(relative_url(relative_path));
    }

    RestClient::response Resource::put(const string& relative_path, const string& payload) {
        return RestClient::put(relative_url(relative_path), content_type, payload);
    }

    RestClient::response Resource::post(const string& relative_path, const string& payload) {
        return RestClient::post(relative_url(relative_path), content_type, payload);
    }

    RestClient::response Resource::del(const string& relative_path) {
        return RestClient::del(relative_url(relative_path));
    }

    string Resource::url() const {
        return _url;
    }

    string Resource::relative_url(const string& relative_path) const {
        return relative_path.empty() ? _url : _url + "/" + relative_path;
    }

    string Resource::base_relative_url(const string& relative_path) const {
        string base = _url.substr(0, _url.find("/"));
        return relative_path.empty() ? base : base + relative_path;
    }

    Document Resource::handle_response(RestClient::response response) const {
        Document doc;
        if (response.code == OK) {
            Json::Reader reader;
            bool parseSuccessful = reader.parse(response.body.c_str(), doc);
            if (!parseSuccessful)
                throw std::runtime_error("[orchestration] Failed parsing response: " + response.body);
        } else if (response.code != NoContent) {
            throw std::runtime_error("[orchestration] Bad response: " + response.body);
        }
        return doc;
    }

} // namespace orchestration
} // namespace mongo

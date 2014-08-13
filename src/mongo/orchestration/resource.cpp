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

        enum Status {
            OK = 200,
            NoContent = 204,
            BadRequest = 400,
            NotFound = 404,
            InternalServerError = 500
        };
    } // namespace

    Resource::Resource(const std::string& url)
        : _url(url)
    {}

    Resource::~Resource() {

    }

    RestClient::response Resource::get(const std::string& relative_path) const {
        return RestClient::get(relativeUrl(relative_path));
    }

    RestClient::response Resource::put(const std::string& relative_path,
            const std::string& payload) {
        return RestClient::put(relativeUrl(relative_path), content_type, payload);
    }

    RestClient::response Resource::post(const std::string& relative_path,
            const std::string& payload) {
        return RestClient::post(relativeUrl(relative_path), content_type, payload);
    }

    RestClient::response Resource::del(const std::string& relative_path) {
        return RestClient::del(relativeUrl(relative_path));
    }

    std::string Resource::url() const {
        return _url;
    }

    std::string Resource::relativeUrl(const std::string& relative_path) const {
        return relative_path.empty() ? _url : _url + "/" + relative_path;
    }

    std::string Resource::baseRelativeUrl(const std::string& relative_path) const {
        std::string base = _url.substr(0, _url.find("/"));
        return relative_path.empty() ? base : base + relative_path;
    }

    Document Resource::handleResponse(const RestClient::response& response) const {
        Document doc;
        if (response.code == OK) {
            Json::Reader reader;
            bool parseSuccessful = reader.parse(response.body.c_str(), doc);
            if (!parseSuccessful)
                throw std::runtime_error("[orchestration] Failed parsing response: " +
                    response.body);
        } else if (response.code != NoContent) {
            throw std::runtime_error("[orchestration] Bad response: " + response.body);
        }
        return doc;
    }

} // namespace orchestration
} // namespace mongo

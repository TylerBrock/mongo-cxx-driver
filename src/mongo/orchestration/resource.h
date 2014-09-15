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

#pragma once

#include <string>
#include <vector>

#include "third_party/jsoncpp/json.h"
#include "third_party/restclient/restclient.h"

// Notes:
//  Posts to resources must not end with a slash or you will get a 404
//  Posts containing a JSON encoded body must conform to strict JSON

namespace mongo {
namespace orchestration {

    using namespace std;

    typedef Json::Value Document;

    class Resource {

    public:
        Resource(const string& url);

    protected:
        RestClient::response get(const string& relative_path="") const;
        RestClient::response put(const string& relative_path="", const string& payload="{}");
        RestClient::response post(const string& relative_path="", const string& payload="{}");
        RestClient::response del(const string& relative_path="");

        RestClient::response action(const string& action);
        const string& url() const;
        string make_url(const string& relative_path) const;
        Document handle_response(RestClient::response response) const;


        template <typename T>
        vector<T> plural_subresource(const string& resource_name) const {
            vector<T> resources;
            Document doc = handle_response(get(resource_name));

            for (unsigned i=0; i<doc.size(); i++) {
                string resource_uri = doc[i]["uri"].asString();
                T resource(_url.substr(0, url().find("/")) + resource_uri);
                resources.push_back(resource);
            }

            return resources;
        }

        template <typename T>
        vector<T> plural_resource(const string& resource_name) const {
            vector<T> resources;

            Document result_array = handle_response(get(resource_name));

            for (unsigned i=0; i<result_array.size(); i++) {
                T temp(make_url(resource_name + "/" + result_array[i].asString()));
                resources.push_back(temp);
            }

            return resources;
        }

    private:
        string _url;

    };

} // namespace orchestration
} // namesace mongo

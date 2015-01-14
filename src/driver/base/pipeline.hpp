// Copyright 2014 MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "driver/config/prelude.hpp"

#include <cstdint>
#include <string>
#include <memory>

#include "bson/document.hpp"

namespace mongo {
namespace driver {

class collection_t;

class LIBMONGOCXX_EXPORT pipeline_t {

   public:
    pipeline_t();

    pipeline_t(pipeline_t&& other) noexcept;
    pipeline_t& operator=(pipeline_t&& rhs) noexcept;

    ~pipeline_t();

    pipeline_t& group(bson::document::view group);
    pipeline_t& limit(std::int32_t limit);
    pipeline_t& match(bson::document::view criteria);
    pipeline_t& out(std::string collection_name);
    pipeline_t& project(bson::document::view projection);
    pipeline_t& redact(bson::document::view restrictions);
    pipeline_t& skip(std::int32_t skip);
    pipeline_t& sort(bson::document::view sort);
    pipeline_t& unwind(std::string field_name);

   private:
    friend collection_t;

    class impl;
    std::unique_ptr<impl> _impl;

}; // class pipeline_t

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"

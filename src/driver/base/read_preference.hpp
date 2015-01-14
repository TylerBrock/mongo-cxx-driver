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
#include "stdx/optional.hpp"

namespace mongo {
namespace driver {

class client_t;
class collection_t;
class database_t;

// TODO: move to own file?
enum class read_mode_t {
    k_primary,
    k_secondary,
    k_primary_preferred,
    k_secondary_preferred,
    k_nearest,
};

class LIBMONGOCXX_EXPORT read_preference_t {

   public:
    explicit read_preference_t(read_mode_t rm = read_mode_t::k_primary);
    read_preference_t(read_mode_t, bson::document::view tags);

    read_preference_t(const read_preference_t&);
    read_preference_t& operator=(const read_preference_t&);

    read_preference_t(read_preference_t&& other) noexcept;
    read_preference_t& operator=(read_preference_t&& rhs) noexcept;

    ~read_preference_t();

    void mode(read_mode_t mode);
    read_mode_t mode() const;

    void tags(bson::document::view tags);
    optional<bson::document::view> tags() const;

   private:
    friend client_t;
    friend collection_t;
    friend database_t;

    class impl;

    read_preference_t(std::unique_ptr<impl>&& implementation);

    std::unique_ptr<impl> _impl;

}; // class read_preference_t

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"

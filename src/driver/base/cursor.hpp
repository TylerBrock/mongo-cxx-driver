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

#include <memory>

#include "bson/document.hpp"

namespace mongo {
namespace driver {

class collection_t;

class LIBMONGOCXX_EXPORT cursor_t {

   public:
    class iterator;

    cursor_t(cursor_t&& other) noexcept;
    cursor_t& operator=(cursor_t&& rhs) noexcept;

    ~cursor_t();

    iterator begin();
    iterator end();

   private:
    friend class collection_t;

    cursor_t(void* cursor_t_ptr);

    class impl;
    std::unique_ptr<impl> _impl;

}; // class cursor_t

class cursor_t::iterator : public std::iterator<
    std::input_iterator_tag,
    bson::document::view
> {

   public:
    const bson::document::view& operator*() const;
    const bson::document::view* operator->() const;

    iterator& operator++();
    void operator++(int);

   private:
    friend cursor_t;
    friend bool operator==(const iterator&, const iterator&);
    friend bool operator!=(const iterator&, const iterator&);

    explicit iterator(cursor_t* cursor_t);

    cursor_t* _cursor_t;
    bson::document::view _doc;

}; // class iterator

bool operator==(const cursor_t::iterator& lhs, const cursor_t::iterator& rhs);
bool operator!=(const cursor_t::iterator& lhs, const cursor_t::iterator& rhs);

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"

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

#include <cstdint>
#include <memory>

#include "bson.h"

#include "driver/base/cursor.hpp"
#include "driver/base/private/cursor.hpp"
#include "driver/private/libmongoc.hpp"

#include "stdx/make_unique.hpp"

namespace mongo {
namespace driver {

cursor_t::cursor_t(void* cursor_t_ptr)
    : _impl(stdx::make_unique<impl>(static_cast<mongoc_cursor_t*>(cursor_t_ptr)))
{}

cursor_t::cursor_t(cursor_t&&) noexcept = default;
cursor_t& cursor_t::operator=(cursor_t&&) noexcept = default;

cursor_t::~cursor_t() = default;

void cursor_t::iterator::operator++(int) {
    operator++();
}

cursor_t::iterator& cursor_t::iterator::operator++() {
    const bson_t* out;
    if (libmongoc::cursor_next(_cursor_t->_impl->cursor_t, &out)) {
        _doc = bson::document::view(bson_get_data(out), out->len);
    } else {
        _cursor_t = nullptr;
    }

    return *this;
}

cursor_t::iterator cursor_t::begin() {
    // Maybe this should be an exception somewhere?
    if (!_impl->cursor_t) {
        return end();
    }
    return iterator(this);
}

cursor_t::iterator cursor_t::end() {
    return iterator(nullptr);
}

cursor_t::iterator::iterator(cursor_t* cursor_t) : _cursor_t(cursor_t)
{
    if (cursor_t) operator++();
}

const bson::document::view& cursor_t::iterator::operator*() const {
    return _doc;
}

const bson::document::view* cursor_t::iterator::operator->() const {
    return &_doc;
}

bool operator==(const cursor_t::iterator& lhs, const cursor_t::iterator& rhs) {
    if (lhs._cursor_t == rhs._cursor_t) return true;
    return &lhs == &rhs;
}

bool operator!=(const cursor_t::iterator& lhs, const cursor_t::iterator& rhs) {
    return !(lhs == rhs);
}

}  // namespace driver
}  // namespace mongo

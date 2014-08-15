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

#include "driver/config/prelude.hpp"
#include "driver/util/unique_ptr_void.hpp"

#include "mongoc.h"

#include "bson/document.hpp"

namespace mongo {
namespace driver {

class collection;

class MONGOCXX_EXPORT cursor {

    friend class collection;

   public:
    class iterator;

    iterator begin();
    iterator end();

   private:
    cursor(void* cursor);

    util::unique_ptr_void _cursor;
};

class cursor::iterator
    : public std::iterator<std::forward_iterator_tag, const bson::document::view&, std::ptrdiff_t,
                           const bson::document::view*, const bson::document::view&> {
    friend class cursor;

   public:
    const bson::document::view& operator*() const;
    const bson::document::view* operator->() const;

    iterator& operator++();

    bool operator==(const iterator& rhs) const;
    bool operator!=(const iterator& rhs) const;

   private:
    iterator(cursor* cursor);

    cursor* _cursor;
    bson::document::view _doc;
    bool _at_end;
};  // class iterator

}  // namespace driver
}  // namespace mongo

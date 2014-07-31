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

#include <cstdint>
#include "util/optional.h"

namespace mongo {
namespace driver {

    class WriteConcern;

    template <class Derived>
    class WriteModel {

    public:
        Derived& write_concern(const WriteConcern* write_concern) {
            _write_concern = &write_concern;
            return *this;
        }

        optional<WriteConcern*> write_concern() const {
            return _write_concern;
        }

    protected:
        optional<const WriteConcern*> _write_concern;

    };

} // namespace driver
} // namespace mongo

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

#include <mongo/bson/builder/array_ctx.hpp>
#include <mongo/bson/builder/value_ctx.hpp>
#include <mongo/bson/builder/single_ctx.hpp>

namespace bson {
namespace builder {

template <class T>
array_ctx<T>::operator single_ctx() {
    return single_ctx(_concrete);
}

template <class T>
value_ctx<T>::operator single_ctx() {
    return single_ctx(_concrete);
}

}  // namespace builder
}  // namespace bson

#include <mongo/driver/config/postlude.hpp>

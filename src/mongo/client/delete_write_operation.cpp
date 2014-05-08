/*    Copyright 2014 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "mongo/client/dbclientinterface.h"
#include "mongo/client/delete_write_operation.h"
#include "mongo/db/namespace_string.h"

namespace {
    const char kCommandKey[] = "delete";
    const char kBatchKey[] = "deletes";
    const char kSelectorKey[] = "q";
    const char kLimitKey[] = "limit";
} // namespace

namespace mongo {

    DeleteWriteOperation::DeleteWriteOperation(const BSONObj selector, int limit)
        : _selector(selector)
        , _limit(limit)
        {}

    DeleteWriteOperation::~DeleteWriteOperation() {
    }

    Operations DeleteWriteOperation::operationType() const {
        return dbDelete;
    }

    void DeleteWriteOperation::startRequest(std::string ns, int flags, BufBuilder* builder) const {
        builder->appendNum(0);
        builder->appendStr(ns);
        builder->appendNum(flags);
    }

    bool DeleteWriteOperation::appendSelfToRequest(int maxSize, BufBuilder* builder) const {
        if (builder->getSize() + _selector.objsize() > maxSize)
            return false;

        _selector.appendSelfToBufBuilder(*builder);
        return true;
    }

    void DeleteWriteOperation::startCommand(std::string ns, int, BSONObjBuilder* builder) const {
        builder->append(kCommandKey, nsToCollectionSubstring(ns));
    }

    bool DeleteWriteOperation::appendSelfToCommand(BSONArrayBuilder* batch) const {
        BSONObjBuilder updateBuilder;
        updateBuilder.append(kSelectorKey, _selector);
        updateBuilder.append(kLimitKey, _limit);
        batch->append(updateBuilder.obj());
        return true;
    }

    void DeleteWriteOperation::endCommand(BSONArrayBuilder* batch, BSONObjBuilder* builder) const {
        builder->append(kBatchKey, batch->arr());
    }

} // namespace mongo

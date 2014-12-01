enable_testing()

set(CMAKE_CXX_FLAGS "-DMONGOCXX_TESTING ${CMAKE_CXX_FLAGS}")

include_directories("include")

macro(test t)
    add_executable(test_${t} test_${t}.cpp)
    target_link_libraries(test_${t} mongocxx-mocked)
endmacro()

test(1)
add_test(test_suite_1 test_1)

add_executable(test_model_update_many
    driver/model/update_many.cpp
)
target_link_libraries(test_model_update_many mongocxx-mocked)
add_test(model_update_many test_model_update_many)

add_executable(test_result_insert_one
    driver/result/insert_one.cpp
)
target_link_libraries(test_result_insert_one mongocxx-mocked)
add_test(result_insert_one test_result_insert_one)

add_executable(test_result_delete
    driver/result/delete.cpp
)
target_link_libraries(test_result_delete mongocxx-mocked)
add_test(result_delete test_result_delete)

add_executable(test_result_replace
    driver/result/replace_one.cpp
)
target_link_libraries(test_result_replace mongocxx-mocked)
add_test(result_replace test_result_replace)

add_executable(test_result_update
    driver/result/update.cpp
)
target_link_libraries(test_result_update mongocxx-mocked)
add_test(result_update test_result_update)

test(3)
add_test(test_suite_3 test_3)
test(4)
add_test(test_suite_4 test_4)
test(5)
add_test(test_suite_5 test_5 10)

add_executable(test_options_aggregate
    driver/options/aggregate.cpp
)
target_link_libraries(test_options_aggregate mongocxx-mocked)
add_test(options_aggregate test_options_aggregate)

add_executable(test_bulk_write
    driver/base/bulk_write.cpp
)
target_link_libraries(test_bulk_write mongocxx-mocked)
add_test(bulk_write test_bulk_write)

add_executable(test_write_concern
    driver/base/write_concern.cpp
)
target_link_libraries(test_write_concern mongocxx-mocked)
add_test(test_write_concern test_write_concern)

add_executable(test_private_write_concern
    driver/base/private/write_concern.cpp
)
target_link_libraries(test_private_write_concern mongocxx-mocked)
add_test(test_private_write_concern test_private_write_concern)

add_executable(test_client
    driver/base/client.cpp
)
target_link_libraries(test_client mongocxx-mocked)
add_test(test_client test_client)

add_executable(test_database
    driver/base/database.cpp
)
target_link_libraries(test_database mongocxx-mocked)
add_test(test_database test_database)

add_executable(test_collection
    driver/base/collection.cpp
)
target_link_libraries(test_collection mongocxx-mocked)
add_test(test_collection test_collection)



add_executable(new_tests
    new_tests.cpp
    bson_builder.cpp
    bson_util_itoa.cpp
    bson_string_or_literal.cpp
)
target_link_libraries(new_tests mongocxx-mocked)

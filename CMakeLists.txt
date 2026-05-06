# @dependency-start
# responsibility Defines the CMake build configuration.
# upstream design cmake/README.md CMake layout guidance
# downstream implementation tests/cpp/prototype/workspace_registry_test.cpp validates editor workspace behavior
# downstream implementation tests/cpp/prototype/duplicate_policy_test.cpp validates duplicate policy behavior
# @dependency-end
cmake_minimum_required(VERSION 3.22)

project(editor_make LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(
    CMAKE_INSTALL_PREFIX
    "${CMAKE_SOURCE_DIR}/.state/cpp-install/default"
    CACHE PATH "Default reusable install prefix for local C++ artifacts." FORCE
  )
endif()

set(
  EDITOR_MAKE_CPP_BUILD_ROOT
  "${CMAKE_SOURCE_DIR}/build/cpp"
  CACHE PATH "Root for out-of-source C++ build trees."
)
set(
  EDITOR_MAKE_CPP_INSTALL_ROOT
  "${CMAKE_SOURCE_DIR}/.state/cpp-install"
  CACHE PATH "Root for reusable local C++ install trees."
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(CTest)

add_library(
  editor_proto
  src/workspace_registry.cpp
  src/duplicate_policy.cpp
)
target_include_directories(editor_proto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(editor_proto PUBLIC cxx_std_20)

add_executable(
  editor_proto_cli
  src/editor_proto_cli.cpp
)
target_link_libraries(editor_proto_cli PRIVATE editor_proto)
install(TARGETS editor_proto_cli RUNTIME DESTINATION bin)

add_executable(
  editor_gui_proto
  src/editor_gui_proto.cpp
)
target_link_libraries(editor_gui_proto PRIVATE editor_proto ${CMAKE_DL_LIBS})
install(TARGETS editor_gui_proto RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  add_test(NAME editor_proto_cli_demo COMMAND editor_proto_cli demo)
  add_test(NAME editor_proto_cli_open_file COMMAND editor_proto_cli open-file /Repo/src/main.cpp)
  add_test(NAME editor_proto_cli_open_dir COMMAND editor_proto_cli open-dir /Work/ProjectA)
  add_test(NAME editor_proto_cli_scroll COMMAND editor_proto_cli scroll 100 -10)
  add_test(NAME editor_proto_cli_scroll_invalid COMMAND editor_proto_cli scroll nope -10)
  add_test(NAME editor_gui_proto_smoke COMMAND editor_gui_proto --smoke)
  set_tests_properties(editor_proto_cli_scroll_invalid PROPERTIES WILL_FAIL TRUE)
endif()


if(BUILD_TESTING)
  add_executable(
    editor_proto_workspace_test
    tests/cpp/prototype/workspace_registry_test.cpp
  )
  target_link_libraries(editor_proto_workspace_test PRIVATE editor_proto)
  add_test(NAME editor_proto_workspace_test COMMAND editor_proto_workspace_test)
endif()


if(BUILD_TESTING)
  add_executable(
    editor_proto_duplicate_policy_test
    tests/cpp/prototype/duplicate_policy_test.cpp
  )
  target_link_libraries(editor_proto_duplicate_policy_test PRIVATE editor_proto)
  add_test(NAME editor_proto_duplicate_policy_test COMMAND editor_proto_duplicate_policy_test)
endif()

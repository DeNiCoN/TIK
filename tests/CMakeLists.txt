include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB tests_SRC
  "*.hpp"
  "*.cpp"
  )

enable_testing()

add_executable(unit_tests
  ${tests_SRC}
)

target_link_libraries(unit_tests
  gtest_main
  tik::shared
)


include(GoogleTest)
gtest_discover_tests(unit_tests)

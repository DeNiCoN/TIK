
file(GLOB shared_SRC
  "*.hpp"
  "*.cpp"
  )

add_library(shared
  ${shared_SRC}
  )

target_include_directories(shared PUBLIC ".")
add_library(tik::shared ALIAS shared)

target_compile_options(shared PUBLIC -fconstexpr-ops-limit=10000000000)

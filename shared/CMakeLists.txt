
file(GLOB shared_SRC
  "*.hpp"
  "*.cpp"
  )

add_library(shared
  ${shared_SRC}
  )

target_include_directories(shared PUBLIC ".")
add_library(huffman_code::shared ALIAS shared)

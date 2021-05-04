cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(tik CXX)

find_package(Boost
  REQUIRED
  COMPONENTS program_options
  )

add_executable(encoder
  encoder.cpp
  )

add_executable(decoder
  decoder.cpp
  )

add_executable(hamming
  hamming.cpp
  )

add_subdirectory(shared)
target_link_libraries(encoder tik::shared Boost::program_options)
target_link_libraries(decoder tik::shared Boost::program_options)
target_link_libraries(hamming tik::shared Boost::program_options)

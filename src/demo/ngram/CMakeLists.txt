project(ngram)


####################################
# src
####################################
file(GLOB_RECURSE ngram_sources
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

file(GLOB_RECURSE demo_headers
  "${ngram_SOURCE_DIR}/../include/*.h")


####################################
# create executable target
####################################
add_executable(ngram
  ${ngram_sources})


####################################
# include directories
####################################
target_include_directories(ngram
  PRIVATE
    $<BUILD_INTERFACE:${lucene++_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${lucene++_SOURCE_DIR}/include/lucene++>
    ${Boost_INCLUDE_DIRS})


####################################
# dependencies
####################################
target_link_libraries(ngram
  PRIVATE
    Boost::boost
    Boost::date_time
    Boost::filesystem
    Boost::iostreams
    Boost::regex
    Boost::thread
    ZLIB::ZLIB
    lucene++::lucene++
    lucene++::lucene++-contrib)

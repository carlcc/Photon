project(photonserver)

file(GLOB_RECURSE SRC_FILES src/*)

add_executable(photonserver ${SRC_FILES})

cmake_minimum_required(VERSION 3.10)
project(BeastMongoApi)

set(CMAKE_CXX_STANDARD 17)

# Adjust these paths accordingly
set(BSONCXX_LIB_DIR "C:/mongo-cxx-driver-install/lib")
set(MONGOCXX_LIB_DIR "C:/mongo-cxx-driver-install/lib")
set(BSONCXX_INCLUDE_DIR "C:/mongo-cxx-driver-install/include/bsoncxx/v_noabi")
set(MONGOCXX_INCLUDE_DIR "C:/mongo-cxx-driver-install/include/mongocxx/v_noabi")

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Threads REQUIRED)

include_directories(${BSONCXX_INCLUDE_DIR} ${MONGOCXX_INCLUDE_DIR})

link_directories(${BSONCXX_LIB_DIR} ${MONGOCXX_LIB_DIR})
add_executable(BeastMongoApi main.cpp)

target_link_libraries(BeastMongoApi
bsoncxx-v_noabi-rhb-x64-v142-md.lib
mongocxx-v_noabi-rhb-x64-v142-md.lib
Boost::system 
Boost::thread 
Threads::Threads
)

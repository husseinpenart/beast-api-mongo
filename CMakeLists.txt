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

# Include directories
include_directories(${BSONCXX_INCLUDE_DIR} ${MONGOCXX_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

link_directories(${BSONCXX_LIB_DIR} ${MONGOCXX_LIB_DIR})
include_directories(src)

add_executable(BeastMongoApi
    src/main.cpp
    src/routes/mainRouter/Router.cpp
    src/controllers/product/ProductController.cpp
    src/models/product/Product.cpp
)
#     src/routes/mainRouter/Router.hpp
#     src/controllers/product/ProductController.cpp


target_link_libraries(BeastMongoApi
    bsoncxx-v_noabi-rhb-x64-v142-md
    mongocxx-v_noabi-rhb-x64-v142-md
    Boost::system
    Boost::thread
    Threads::Threads
)

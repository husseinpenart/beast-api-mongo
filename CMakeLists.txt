cmake_minimum_required(VERSION 3.10)
project(BeastMongoApi)

set(CMAKE_CXX_STANDARD 17)

# Paths for MongoDB C++ driver
set(BSONCXX_LIB_DIR "C:/mongo-cxx-driver-install/lib")
set(MONGOCXX_LIB_DIR "C:/mongo-cxx-driver-install/lib")
set(BSONCXX_INCLUDE_DIR "C:/mongo-cxx-driver-install/include/bsoncxx/v_noabi")
set(MONGOCXX_INCLUDE_DIR "C:/mongo-cxx-driver-install/include/mongocxx/v_noabi")

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${BSONCXX_INCLUDE_DIR}
    ${MONGOCXX_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    src
    src/base
    src/controllers/product
    src/controllers/user
    src/models/product
    src/models/user
    src/routes/mainRouter
    src/routes/users
    src/routes/postRoute
    src/models/blog
    src/controllers/post
)

# Link directories
link_directories(${BSONCXX_LIB_DIR} ${MONGOCXX_LIB_DIR})

# Add executable
add_executable(BeastMongoApi
    src/main.cpp
    src/routes/mainRouter/router.cpp
    src/routes/products/product_routes.cpp
    src/routes/users/UserRouter.cpp
    src/controllers/product/ProductController.cpp
    src/controllers/user/UserController.cpp
    src/models/product/Product.cpp
    src/models/user/User.cpp
    src/models/blog/blogModel.cpp
    src/controllers/post/postContrllers.cpp
    src/routes/postRoute/post_route.cpp
)

# Link libraries
target_link_libraries(BeastMongoApi
    bsoncxx-v_noabi-rhb-x64-v142-md
    mongocxx-v_noabi-rhb-x64-v142-md
    Boost::system
    Boost::thread
    Threads::Threads
    bcrypt
)
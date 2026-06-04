cmake_minimum_required(VERSION 3.22)
project (TruePass-server LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(jsoncpp CONFIG REQUIRED)

find_package(Drogon CONFIG REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/controllers/AuthController.cpp 
    src/controllers/AuthController.h 
    
    src/middlewares/SessionLockMiddleware.h 
    src/utils/ResponseBuilder.h 
    src/utils/ResponseBuilder.cpp 
    src/utils/RequestReader.h 
    "src/dao/UserDao.h" 
    "src/dao/BaseDao.h" 
    "src/dao/UserDao.cpp" "src/services/OpaqueService.h" "src/services/OpaqueService.cpp" "src/utils/StartupEntry.h" "src/services/SessionService.h" "src/services/SessionService.cpp" "src/models/UserSession.h" "src/controllers/UserController.h" "src/controllers/UserController.cpp" "src/middlewares/AuthFilter.h" "src/middlewares/AuthFilter.cpp" "src/dao/VaultDao.h" "src/dao/VaultDao.cpp" "src/controllers/VaultController.h" "src/controllers/VaultController.cpp" "src/dao/VaultItemDao.h" "src/dao/VaultItemDao.cpp" "src/controllers/VaultItemController.h" "src/controllers/VaultItemController.cpp" "src/models/DeviceBoundContext.h" "src/models/DeviceBoundContext.cpp")

set(AUTH_DEPS "")
list(APPEND AUTH_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/AuthController.h")
list(APPEND AUTH_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/src/middlewares/SessionLockMiddleware.h")
list(APPEND AUTH_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/ResponseBuilder.h")
list(APPEND AUTH_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RequestReader.h")

set_source_files_properties(src/controllers/AuthController.cpp 
    PROPERTIES OBJECT_DEPENDS "${AUTH_DEPS}"
)

#set_source_files_properties(src/controllers/AuthController.cpp PROPERTIES
#    OBJECT_DEPENDS
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/AuthController.h;${CMAKE_CURRENT_SOURCE_DIR}/src/middlewares/SessionLockMiddleware.h;${CMAKE_CURRENT_SOURCE_DIR}/src/utils/ResponseBuilder.h;${CMAKE_CURRENT_SOURCE_DIR}/src/utils/RequestReader.h")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    JsonCpp::JsonCpp
    Drogon::Drogon
    TruePass-common)

# Copy the config.json file to the build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
    "${CMAKE_CURRENT_BINARY_DIR}/config.json"
    COPYONLY
)

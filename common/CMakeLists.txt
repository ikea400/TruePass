cmake_minimum_required(VERSION 3.6)
project(TruePass-common)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glaze CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
  opaquepp
  GIT_REPOSITORY https://github.com/ikea400/opaquepp.git
)
FetchContent_MakeAvailable(opaquepp)

set(PROJECT_SOURCES
    "include/dto/auth_dto.h"
    "include/utils/BinEncoding.h"
    "src/utils/fast-hex/hex.h"
    "src/utils/fast-hex/hex.cpp"
    "src/utils/base64/base64.h" 
    "include/utils/BinArray.h" 
    "include/utils/BinVector.h" 
    "include/utils/BoundedValue.h" 
    "include/dto/response_base.h" 
    "include/utils/uuid.h" 
    "include/utils/utils.h" 
    "src/utils/utils.cpp" 
    "include/utils/validation.h" 
    "include/utils/validation.cpp" 
    "include/utils/SharedString.h"
    "include/crypto/cipher.h" 
    "src/crypto/cipher.cpp" 
    "include/crypto/hasher.h" 
    "src/crypto/hasher.cpp"  
    "include/crypto/hash_algo.h" 
    "src/crypto/hash_algo.cpp" 
    "include/crypto/hmac.h" 
    "include/crypto/hkdf.h" 
    "include/crypto/argon2.h" 
    "src/crypto/hmac.cpp" 
    "src/crypto/hkdf.cpp" 
    "src/crypto/argon2.cpp" 
    "src/crypto/kmac.cpp" 
    "include/crypto/kmac.h" 
    "include/utils/SecureArray.h" 
    "include/dto/user_dto.h" 
    "include/dto/vault_dto.h" 
    "include/dto/vault_item_dto.h" 
    "include/utils/Timer.h" 
    "include/utils/ScopedTimer.h" "include/utils/CryptoRandomizer.h" "src/utils/CryptoRandomizer.cpp" "include/utils/Randomizer.h" "include/utils/SecretRandomizer.h" "src/utils/fast-hex/SecretRandomizer.cpp" "include/dto/device_bound_dto.h" "include/crypto/curves.h" "src/crypto/curves.cpp" )

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    glaze::glaze 
    ctre::ctre
    opaquepp
    OpenSSL::Crypto
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
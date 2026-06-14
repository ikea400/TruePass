cmake_minimum_required(VERSION 3.22)
project (TruePass LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Global settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force Address Space Layout Randomization (ASLR / PIE) globally
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CTest)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -fstack-protector-strong     # Protects the stack layout using canaries
        -fstack-clash-protection     # Prevents stack-clash page skipping attacks
        -Wall -Wextra -Wpedantic     # Enforce strict warning standards
    )
    
    # Enable buffer runtime safety checks (Requires optimization like -O2 / Release)
    add_compile_definitions(_FORTIFY_SOURCE=3)
    
    # Linker Hardening: Read-Only Relocations (RELRO) and Immediate Binding
    add_link_options(-Wl,-z,relro -Wl,-z,now)

elseif(MSVC)
    add_compile_options(
        /GS                          # Buffer Security Check (Stack Canaries)
        /guard:cf                    # Control Flow Guard (CFG) Compiler check
        /W4                          # High warning level
    )
    
    add_link_options(
        /GUARD:CF                    # Control Flow Guard Linker verification
        /DYNAMICBASE                 # Explicitly enforce ASLR
        /NXCOMPAT                    # Data Execution Prevention (DEP / No-Execute)
    )
endif()

enable_testing()

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

# Add the sub-projects
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
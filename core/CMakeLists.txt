project(ac_core VERSION 3.0.0.0 LANGUAGES CXX)

set(CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# third-party
include(${DEPENDENCY_DIR}/ruapu.cmake)
include(${DEPENDENCY_DIR}/stb.cmake)
if(AC_CORE_WITH_OPENCL)
    include(${DEPENDENCY_DIR}/opencl.cmake)
    include(${CMAKE_DIR}/MinifierOpenCLKernel.cmake)
endif()
if(AC_CORE_WITH_EIGEN3)
    include(${DEPENDENCY_DIR}/eigen3.cmake)
endif()
if(AC_CORE_WITH_CUDA)
    include(${DEPENDENCY_DIR}/cuda.cmake)
endif()

if(AC_SHARED_LIB)
    add_library(ac SHARED)
else()
    add_library(ac STATIC)
endif()
add_library(AC::Core ALIAS ac)

target_sources(ac PRIVATE
    ${CORE_SOURCE_DIR}/src/Alloc.cpp
    ${CORE_SOURCE_DIR}/src/Image.cpp
    ${CORE_SOURCE_DIR}/src/ImageProcess.cpp
    ${CORE_SOURCE_DIR}/src/ImageIO.cpp
    ${CORE_SOURCE_DIR}/src/Model.cpp
    ${CORE_SOURCE_DIR}/src/Processor.cpp
    ${CORE_SOURCE_DIR}/src/cpu/Dispatch.cpp
    ${CORE_SOURCE_DIR}/src/cpu/CPUProcessor.cpp
    ${CORE_SOURCE_DIR}/src/cpu/Generic.cpp
    $<$<BOOL:${AC_CORE_WITH_EIGEN3}>:${CORE_SOURCE_DIR}/src/cpu/Eigen3.cpp>
    $<$<BOOL:${AC_CORE_WITH_SSE}>:${CORE_SOURCE_DIR}/src/cpu/x86/SSE.cpp>
    $<$<BOOL:${AC_CORE_WITH_AVX}>:${CORE_SOURCE_DIR}/src/cpu/x86/AVX.cpp>
    $<$<BOOL:${AC_CORE_WITH_NEON}>:${CORE_SOURCE_DIR}/src/cpu/arm/NEON.cpp>
    $<$<BOOL:${AC_CORE_WITH_WASM_SIMD128}>:${CORE_SOURCE_DIR}/src/cpu/wasm/SIMD128.cpp>
    $<$<BOOL:${AC_CORE_WITH_OPENCL}>:${CORE_SOURCE_DIR}/src/opencl/OpenCLProcessor.cpp>
    $<$<BOOL:${AC_CORE_WITH_CUDA}>:${CORE_SOURCE_DIR}/src/cuda/CUDAProcessor.cpp>
    $<$<BOOL:${AC_CORE_WITH_CUDA}>:${CORE_SOURCE_DIR}/src/cuda/Kernel.cu>
)

target_include_directories(ac PUBLIC
    $<BUILD_INTERFACE:${CORE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CORE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:ac/include>
)

# We don't need setting any cpu arch flag for MSVC, setting it may actually degrade performance.
# But we must set cpu arch flag for ClangCL.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_SIMULATE_ID MATCHES "MSVC" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
    if(AC_CORE_WITH_SSE AND BUILD_ARCH_32BIT)
        set_source_files_properties(${CORE_SOURCE_DIR}/src/cpu/x86/SSE.cpp PROPERTIES COMPILE_OPTIONS "/arch:SSE")
    endif()
    if(AC_CORE_WITH_AVX)
        set_source_files_properties(${CORE_SOURCE_DIR}/src/cpu/x86/AVX.cpp PROPERTIES COMPILE_OPTIONS "$<IF:$<BOOL:${AC_CORE_WITH_FMA}>,/arch:AVX2,/arch:AVX>")
    endif()
    if(AC_CORE_WITH_NEON)
        set_source_files_properties(${CORE_SOURCE_DIR}/src/cpu/arm/NEON.cpp PROPERTIES COMPILE_OPTIONS "/arch:armv8.0")
    endif()
elseif(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(AC_CORE_WITH_SSE)
        set_source_files_properties(${CORE_SOURCE_DIR}/src/cpu/x86/SSE.cpp PROPERTIES COMPILE_OPTIONS "-msse")
    endif()
    if(AC_CORE_WITH_AVX)
        set_source_files_properties(${CORE_SOURCE_DIR}/src/cpu/x86/AVX.cpp PROPERTIES COMPILE_OPTIONS "-mavx;$<$<BOOL:${AC_CORE_WITH_FMA}>:-mfma>")
    endif()
    if(AC_CORE_WITH_NEON)
        set_source_files_properties(${CORE_SOURCE_DIR}/src/cpu/arm/NEON.cpp PROPERTIES COMPILE_OPTIONS "$<IF:$<BOOL:${BUILD_ARCH_32BIT}>,-mfpu=neon,-march=armv8-a>")
    endif()
    if(AC_CORE_WITH_WASM_SIMD128)
        set_source_files_properties(${CORE_SOURCE_DIR}/src/cpu/wasm/SIMD128.cpp PROPERTIES COMPILE_OPTIONS "-msimd128")
    endif()
endif()

# aligned malloc
include(CheckIncludeFileCXX)
include(CheckCXXSymbolExists)
check_include_file_cxx(malloc.h AC_CORE_HAVE_MALLOC_H)
if (AC_CORE_HAVE_MALLOC_H)
    check_cxx_symbol_exists(_aligned_malloc malloc.h AC_CORE_HAVE_WIN32_ALIGNED_MALLOC)
    check_cxx_symbol_exists(_aligned_free malloc.h AC_CORE_HAVE_WIN32_ALIGNED_FREE)
    check_cxx_symbol_exists(memalign malloc.h AC_CORE_HAVE_BSD_MEMALIGN)
endif()
check_cxx_symbol_exists(std::aligned_alloc cstdlib AC_CORE_HAVE_STD_ALIGNED_ALLOC)
check_cxx_symbol_exists(posix_memalign stdlib.h AC_CORE_HAVE_POSIX_MEMALIGN)

target_link_libraries(ac PRIVATE
    ac_util_misc ac_util_threads ac_util_parallel
    dep::ruapu
    dep::stb
    $<$<BOOL:${AC_CORE_WITH_EIGEN3}>:dep::eigen3>
    $<$<BOOL:${AC_CORE_WITH_OPENCL}>:dep::opencl>
    $<$<BOOL:${AC_CORE_WITH_CUDA}>:dep::cuda>
)

set(AC_CORE_FEATURE_LIST
    generic
    $<$<BOOL:${AC_CORE_WITH_EIGEN3}>:eigen3>
    $<$<BOOL:${AC_CORE_WITH_SSE}>:sse>
    $<$<BOOL:${AC_CORE_WITH_AVX}>:avx>
    $<$<BOOL:${AC_CORE_WITH_FMA}>:fma>
    $<$<BOOL:${AC_CORE_WITH_NEON}>:neon>
    $<$<BOOL:${AC_CORE_WITH_WASM_SIMD128}>:wasm_simd128>
    $<$<BOOL:${AC_CORE_WITH_OPENCL}>:opencl>
    $<$<BOOL:${AC_CORE_WITH_CUDA}>:cuda>
)

target_compile_definitions(ac PUBLIC
    $<$<BOOL:${AC_CORE_WITH_EIGEN3}>:AC_CORE_WITH_EIGEN3>
    $<$<BOOL:${AC_CORE_WITH_SSE}>:AC_CORE_WITH_SSE>
    $<$<BOOL:${AC_CORE_WITH_AVX}>:AC_CORE_WITH_AVX>
    $<$<BOOL:${AC_CORE_WITH_FMA}>:AC_CORE_WITH_FMA>
    $<$<BOOL:${AC_CORE_WITH_NEON}>:AC_CORE_WITH_NEON>
    $<$<BOOL:${AC_CORE_WITH_WASM_SIMD128}>:AC_CORE_WITH_WASM_SIMD128>
    $<$<BOOL:${AC_CORE_WITH_OPENCL}>:AC_CORE_WITH_OPENCL>
    $<$<BOOL:${AC_CORE_WITH_CUDA}>:AC_CORE_WITH_CUDA>
    $<$<BOOL:${AC_CORE_DISABLE_IMAGE_IO}>:AC_CORE_DISABLE_IMAGE_IO>
    $<$<AND:$<BOOL:${AC_CORE_HAVE_WIN32_ALIGNED_MALLOC}>,$<BOOL:${AC_CORE_HAVE_WIN32_ALIGNED_FREE}>>:AC_CORE_HAVE_WIN32_ALIGNED_MALLOC>
    $<$<BOOL:${AC_CORE_HAVE_STD_ALIGNED_ALLOC}>:AC_CORE_HAVE_STD_ALIGNED_ALLOC>
    $<$<BOOL:${AC_CORE_HAVE_POSIX_MEMALIGN}>:AC_CORE_HAVE_POSIX_MEMALIGN>
    $<$<BOOL:${AC_CORE_HAVE_BSD_MEMALIGN}>:AC_CORE_HAVE_BSD_MEMALIGN>

    AC_CORE_MALLOC_ALIGN=$<IF:$<BOOL:${AC_CORE_WITH_AVX}>,32,16>

    AC_CORE_VERSION_STR="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
    AC_CORE_FEATURES="$<JOIN:${AC_CORE_FEATURE_LIST}, >"
)

if(AC_CORE_ENABLE_FAST_MATH)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_SIMULATE_ID MATCHES "MSVC" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))
        target_compile_options(ac PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/fp:fast>)
    else()
        target_compile_options(ac PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-ffast-math>)
    endif()
endif()

if(EMSCRIPTEN)
    target_link_options(ac PUBLIC -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB)
endif()

ac_check_disable_flags(ac)
ac_check_enable_static_crt(ac)
ac_check_disable_pic(ac)

if(AC_CORE_WITH_OPENCL)
    file(WRITE "${CORE_BINARY_DIR}/filegen/include/AC/Core/OpenCL/Kernel.cl.hpp.in" [[
#ifndef AC_CORE_OPENCL_KERNEL_HPP
#define AC_CORE_OPENCL_KERNEL_HPP
namespace ac::core::opencl
{
    constexpr const char* KernelString = R"(@AC_CORE_OPENCL_KERNEL_CL_CONTENT_MINIFIED@)";
}
#endif]])

    file(READ "${CORE_SOURCE_DIR}/src/opencl/Kernel.cl" AC_CORE_OPENCL_KERNEL_CL_CONTENT)
    minifier_opencl_kernel(AC_CORE_OPENCL_KERNEL_CL_CONTENT AC_CORE_OPENCL_KERNEL_CL_CONTENT_MINIFIED)

    configure_file(
        ${CORE_BINARY_DIR}/filegen/include/AC/Core/OpenCL/Kernel.cl.hpp.in
        ${CORE_BINARY_DIR}/include/AC/Core/OpenCL/Kernel.hpp
        @ONLY
    )
endif()

set_target_properties(ac PROPERTIES EXPORT_NAME "Core")

include(GenerateExportHeader)
generate_export_header(ac
    BASE_NAME "AC"
    EXPORT_FILE_NAME ${CORE_BINARY_DIR}/include/ACExport.hpp
)

install(
    TARGETS ac EXPORT AC
    ARCHIVE DESTINATION ac/lib
    LIBRARY DESTINATION ac/lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CORE_SOURCE_DIR}/include ${CORE_BINARY_DIR}/include DESTINATION ac)

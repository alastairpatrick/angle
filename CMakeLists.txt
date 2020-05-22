cmake_minimum_required(VERSION 3.8)
project(angle CXX C)
include(config.cmake)

option(USE_FRAME_CAPTURE "build with frame capture support" OFF)
option(USE_LOADER "build with EGL/GLESv2 loader" ON)
option(USE_VULKAN "build with vulkan support" OFF)
option(USE_METAL "build with metal support" OFF)
if(WINDOWS_DESKTOP AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES arm)
  option(USE_D3D9 "build with d3d9 support" ON)
  option(USE_GL "build with native opengl/es support via wgl" ON)
endif()
    

find_package(GTest MODULE REQUIRED)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(X11 QUIET)

enable_testing()
include(UseJava)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_definitions(
    -D_CRT_SECURE_NO_DEPRECATE
    -D_SCL_SECURE_NO_WARNINGS
    -D_HAS_EXCEPTIONS=0
    -DNOMINMAX
    -DANGLE_STANDALONE_BUILD
    -DANGLE_ENABLE_DEBUG_ANNOTATIONS
)

include_directories(include src ${CMAKE_CURRENT_BINARY_DIR}/include)

if(MSVC)
    set(ANGLE_IGNORE_WARNINGS "/wd4200" "/wd4267" "/wd4305")
elseif(APPLE)
    set(ANGLE_IGNORE_WARNINGS "-DGL_SILENCE_DEPRECATION")
else()
    set(ANGLE_IGNORE_WARNINGS "")
endif()

##########
# angle::common
# android_util.h/cpp is required and not using android apis
if(WINDOWS_DESKTOP)
    set(ANGLE_COMMON_PLATFORM_FILTER "_winuwp|_linux|_mac|_posix")
elseif(WINDOWS_STORE)
    set(ANGLE_COMMON_PLATFORM_FILTER "_win|_linux|_mac|_posix")
elseif(LINUX)
    set(ANGLE_COMMON_PLATFORM_FILTER "_win|_mac")
elseif(APPLE)
    set(ANGLE_COMMON_PLATFORM_FILTER "_linux|_win")
endif()
file(GLOB ANGLE_COMMON_SOURCES
    "src/common/*.h"
    "src/common/*.inl"
    "src/common/*.cpp"
    "src/common/third_party/base/anglebase/*.h"
    "src/common/third_party/base/anglebase/*.cc"
    "src/common/third_party/base/anglebase/containers/*.h"
    "src/common/third_party/base/anglebase/numerics/*.h"
    "src/common/third_party/base/anglebase/numerics/*.cc"
    "src/common/third_party/xxhash/*.h"
    "src/common/third_party/xxhash/*.c"
    "src/common/third_party/smhasher/src/*.h"
    "src/common/third_party/smhasher/src/*.cpp")
list(FILTER ANGLE_COMMON_SOURCES EXCLUDE REGEX "_unittest|event_tracer|${ANGLE_COMMON_PLATFORM_FILTER}")
add_library(angle_common STATIC ${ANGLE_COMMON_SOURCES})
target_include_directories(angle_common PUBLIC src/common/third_party/base)
target_link_libraries(angle_common ${CMAKE_DL_LIBS})
add_library(angle::common ALIAS angle_common)

# These settings are applied to all ANGLE modules on account of PUBLIC.
target_compile_options(angle_common PUBLIC ${ANGLE_IGNORE_WARNINGS})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_compile_definitions(angle_common PUBLIC -DANGLE_IS_64_BIT_CPU)
else()
  target_compile_definitions(angle_common PUBLIC -DANGLE_IS_32_BIT_CPU)
  if(MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES 86)
    target_compile_options(angle_common PUBLIC -arch:SSE2) # clang-cl, loadimage.cpp
  endif()
endif()


##########
# angle::image_util
file(GLOB ANGLE_IMAGE_UTIL_SOURCES
    "src/image_util/*.h"
    "src/image_util/*.inl"
    "src/image_util/*.cpp"
)
add_library(angle_image_util STATIC ${ANGLE_IMAGE_UTIL_SOURCES})
target_link_libraries(angle_image_util PRIVATE angle::common)
add_library(angle::image_util ALIAS angle_image_util)


##########
# angle::gpu_info_util
set(ANGLE_GPU_INFO_UTIL_SOURCES src/gpu_info_util/SystemInfo.cpp)
if(APPLE)
  list(APPEND ANGLE_GPU_INFO_UTIL_SOURCES src/gpu_info_util/SystemInfo_macos.mm)
elseif(WIN32)
  list(APPEND ANGLE_GPU_INFO_UTIL_SOURCES src/gpu_info_util/SystemInfo_win.cpp)
elseif(ANDROID)
  list(APPEND ANGLE_GPU_INFO_UTIL_SOURCES src/gpu_info_util/SystemInfo_android.cpp)
else()
  list(APPEND ANGLE_GPU_INFO_UTIL_SOURCES src/gpu_info_util/SystemInfo_linux.cpp)
endif()
add_library(angle_gpu_info_util STATIC ${ANGLE_GPU_INFO_UTIL_SOURCES})
target_link_libraries(angle_gpu_info_util PRIVATE angle::common)
add_library(angle::gpu_info_util ALIAS angle_gpu_info_util)


##########
# angle::translator
file(GLOB_RECURSE TRANSLATOR_SOURCES
    "src/compiler/translator/glslang.l"
    "src/compiler/translator/glslang.y"
    "src/compiler/translator/*.h"
    "src/compiler/translator/*.cpp"
    "src/third_party/compiler/ArrayBoundsClamper.cpp"
)
list(FILTER TRANSLATOR_SOURCES EXCLUDE REGEX "_unittest")
list(FILTER TRANSLATOR_SOURCES EXCLUDE REGEX "ESSL|essl")
if(NOT WIN32)
    list(FILTER TRANSLATOR_SOURCES EXCLUDE REGEX "HLSL|hlsl")
endif()
list(FILTER TRANSLATOR_SOURCES EXCLUDE REGEX "Vulkan")
add_library(angle_translator STATIC ${TRANSLATOR_SOURCES})
target_compile_definitions(angle_translator PRIVATE
    #-DANGLE_ENABLE_ESSL
    -DANGLE_ENABLE_GLSL
)
if(WIN32)
    target_compile_definitions(angle_translator PRIVATE -DANGLE_ENABLE_HLSL)
endif()
target_link_libraries(angle_translator PRIVATE angle::common)
add_library(angle::translator ALIAS angle_translator)


##########
# angle::preprocessor
file(GLOB PREPROCESSOR_SOURCES
    "src/compiler/preprocessor/*.h"
    "src/compiler/preprocessor/*.cpp"
)
add_library(angle_preprocessor STATIC ${PREPROCESSOR_SOURCES})
target_link_libraries(angle_preprocessor PRIVATE angle::common)
add_library(angle::preprocessor ALIAS angle_preprocessor)


##########
# libANGLE

## OpenGL Renderer
if(USE_GL AND WINDOWS_DESKTOP OR LINUX OR APPLE)
    if(WINDOWS_DESKTOP)
        set(ANGLE_RENDERER_OPENGL_PLATFORM 
            "src/libANGLE/renderer/gl/wgl/*.cpp"
            "src/libANGLE/renderer/gl/wgl/*.inl"
            "src/libANGLE/renderer/gl/wgl/*.h"
        )
    elseif(APPLE)
        set(ANGLE_RENDERER_OPENGL_PLATFORM 
            "src/libANGLE/renderer/gl/cgl/*.mm"
            "src/libANGLE/renderer/gl/cgl/*.cpp"
            "src/libANGLE/renderer/gl/cgl/*.inl"
            "src/libANGLE/renderer/gl/cgl/*.h"
        )
    elseif(LINUX)
        set(ANGLE_RENDERER_OPENGL_PLATFORM 
            "src/libANGLE/renderer/gl/egl/*.cpp"
            "src/libANGLE/renderer/gl/egl/*.inl"
            "src/libANGLE/renderer/gl/egl/*.h"
            "src/libANGLE/renderer/gl/glx/*.cpp"
            "src/libANGLE/renderer/gl/glx/*.inl"
            "src/libANGLE/renderer/gl/glx/*.h"
        )
    endif()

    file(GLOB LIBANGLE_GL_SOURCES
        "src/libANGLE/renderer/gl/*.cpp"
        "src/libANGLE/renderer/gl/*.inl"
        "src/libANGLE/renderer/gl/*.h"

        ${ANGLE_RENDERER_OPENGL_PLATFORM}
    )
    list(FILTER LIBANGLE_GL_SOURCES EXCLUDE REGEX "_unittest")
    add_library(angle_renderer_opengl INTERFACE)
    target_sources(angle_renderer_opengl INTERFACE ${LIBANGLE_GL_SOURCES})
    target_compile_definitions(angle_renderer_opengl INTERFACE -DANGLE_ENABLE_OPENGL)
    add_library(angle::renderer::opengl ALIAS angle_renderer_opengl)
endif()

# D3D Renderers
if(WIN32)
    ## All D3D Sources
    file(GLOB_RECURSE LIBANGLE_D3D_SOURCES
        "src/libANGLE/renderer/d3d/*.cpp"
        "src/libANGLE/renderer/d3d/*.inl"
        "src/libANGLE/renderer/d3d/*.h"
    )
    list(FILTER LIBANGLE_D3D_SOURCES EXCLUDE REGEX "_unittest")

    ## WinRT D3D Renderer
    if(WINDOWS_STORE)
        set(LIBANGLE_D3D_WINRT_SOURCES ${LIBANGLE_D3D_SOURCES})
        list(FILTER LIBANGLE_D3D_WINRT_SOURCES INCLUDE REGEX "winrt")
        add_library(angle_renderer_winrt INTERFACE)
        target_sources(angle_renderer_winrt INTERFACE ${LIBANGLE_D3D_WINRT_SOURCES})
        add_library(angle::renderer::winrt ALIAS angle_renderer_winrt)
    endif()

    ## Win32/d3d9 D3D Renderer
    if(WINDOWS_DESKTOP)
        set(LIBANGLE_D3D_DESKTOP_SOURCES ${LIBANGLE_D3D_SOURCES})
        if(USE_D3D9)
            list(FILTER LIBANGLE_D3D_DESKTOP_SOURCES INCLUDE REGEX "d3d9|win32|converged")
        else()
            list(FILTER LIBANGLE_D3D_DESKTOP_SOURCES INCLUDE REGEX "win32|converged")
        endif()
        add_library(angle_renderer_win32 INTERFACE)
        target_sources(angle_renderer_win32 INTERFACE ${LIBANGLE_D3D_DESKTOP_SOURCES})
        if(USE_D3D9)
            target_compile_definitions(angle_renderer_win32 INTERFACE -DANGLE_ENABLE_D3D9)
            target_link_libraries(angle_renderer_win32 INTERFACE d3d9)
        endif()
        target_link_libraries(angle_renderer_win32 INTERFACE user32 gdi32)
        add_library(angle::renderer::win32 ALIAS angle_renderer_win32)
        if(CMAKE_HOST_WIN32)
            set_property(SOURCE src/libANGLE/renderer/d3d/HLSLCompiler.cpp APPEND PROPERTY COMPILE_FLAGS "-DANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES={\\\"d3dcompiler_47.dll\\\",\\\"d3dcompiler_46.dll\\\",\\\"d3dcompiler_43.dll\\\"}")
        else() # do not let bash expend {xx,oo}
            set_property(SOURCE src/libANGLE/renderer/d3d/HLSLCompiler.cpp APPEND PROPERTY COMPILE_FLAGS "-DANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES='{\"d3dcompiler_47.dll\",\"d3dcompiler_46.dll\",\"d3dcompiler_43.dll\"}'")
        endif()
    endif()

    ## D3D11 Base renderer
    list(FILTER LIBANGLE_D3D_SOURCES EXCLUDE REGEX "winrt|d3d9|win32") #|converged
    find_library(DXGUID_LIB NAMES dxguid)
    find_library(D3D11_LIB NAMES d3d11)
    add_library(angle_renderer_d3d INTERFACE)
    target_sources(angle_renderer_d3d INTERFACE ${LIBANGLE_D3D_SOURCES})
    target_compile_definitions(angle_renderer_d3d INTERFACE
        -DANGLE_ENABLE_D3D11
    )
    target_link_libraries(angle_renderer_d3d INTERFACE d3d11 dxgi dxguid) #${D3D11_LIB} ${DXGUID_LIB})
    add_library(angle::renderer::d3d ALIAS angle_renderer_d3d)
endif()

## Core libANGLE library
if(WIN32)
    set(LIBANGLE_SOURCES_PLATFORM
        "src/third_party/systeminfo/SystemInfo.cpp"
    )
elseif(APPLE)
    file(GLOB LIBANGLE_SOURCES_PLATFORM
         "src/libANGLE/renderer/*.mm"
    )
else()
    set(LIBANGLE_SOURCES_PLATFORM )
endif()

file(GLOB LIBANGLE_SOURCES
    "src/common/third_party/murmurhash/MurmurHash3.cpp"
    "src/common/event_tracer.cpp"

    "src/libANGLE/*.cpp"
    "src/libANGLE/*.inl"
    "src/libANGLE/*.h"

    "src/libANGLE/renderer/*.cpp"
    "src/libANGLE/renderer/*.inl"
    "src/libANGLE/renderer/*.h"

    #"src/libANGLE/renderer/null/*.cpp"
    #"src/libANGLE/renderer/null/*.inl"
    #"src/libANGLE/renderer/null/*.h"

    ${LIBANGLE_SOURCES_PLATFORM}
)
list(FILTER LIBANGLE_SOURCES EXCLUDE REGEX "_unittest")
if(USE_FRAME_CAPTURE)
    list(FILTER LIBANGLE_SOURCES EXCLUDE REGEX "FrameCapture_mock.cpp")
else()
    list(FILTER LIBANGLE_SOURCES EXCLUDE REGEX "FrameCapture.cpp")
endif()
if(NOT USE_VULKAN AND NOT USE_METAL)
    list(FILTER LIBANGLE_SOURCES EXCLUDE REGEX "glslang_wrapper_utils.cpp") # need glslang source
endif()
if(LINUX OR APPLE)
    set(LIBANGLE_RENDERER_PLATFORM
        angle::renderer::opengl
    )
elseif(WINDOWS_STORE)
    set(LIBANGLE_RENDERER_PLATFORM
        angle::renderer::d3d
        angle::renderer::winrt
    )
elseif(WINDOWS_DESKTOP)
    set(LIBANGLE_RENDERER_PLATFORM
        angle::renderer::d3d
        angle::renderer::win32
    )
    if(USE_GL)
        list(APPEND LIBANGLE_RENDERER_PLATFORM angle::renderer::opengl)
    endif()
else()
    set(LIBANGLE_RENDERER_PLATFORM )
endif()

if(LINUX)
    set(LIBANGLE_RENDERER_COMPILEDEF
        -DANGLE_USE_X11
    )
else()
    set(LIBANGLE_RENDERER_COMPILEDEF )
endif()

add_library(libANGLE STATIC ${LIBANGLE_SOURCES})
target_link_libraries(libANGLE PRIVATE
    angle::common
    angle::gpu_info_util
    angle::image_util
    angle::translator
    angle::preprocessor
    ${LIBANGLE_RENDERER_PLATFORM}
    ${X11_LIBRARIES}
)
target_include_directories(libANGLE PRIVATE "src/third_party/khronos")
target_compile_definitions(libANGLE
    PRIVATE #-DANGLE_ENABLE_NULL
            -DLIBANGLE_IMPLEMENTATION 
            ${LIBANGLE_RENDERER_COMPILEDEF}
           -DANGLE_CAPTURE_ENABLED=$<BOOL:${USE_FRAME_CAPTURE}>
    PUBLIC -DANGLE_PLATFORM_EXPORT=
           -DANGLE_EXPORT=
)
if (MSVC)
    # To fix Error	C1128	number of sections exceeded object file format limit: compile with /bigobj	D:\src\angle\out\build\x64-Debug\angle	D:\src\angle\src\libANGLE\capture_gles_ext_autogen.cpp	1	
    target_compile_options(libANGLE PRIVATE /bigobj)
endif()
add_library(angle::libANGLE ALIAS libANGLE)


##########
# libGLESv2
file(GLOB LIBGLESV2_SOURCES
    "src/libGLESv2/*.h"
    "src/libGLESv2/*.cpp"
    "src/libGLESv2/libGLESv2_autogen.def"
)
list(FILTER LIBGLESV2_SOURCES EXCLUDE REGEX "proc_table_autogen|proc_table_gles_autogen") # unused, and duplicated symbols

add_library(libGLESv2 STATIC ${LIBGLESV2_SOURCES})
target_link_libraries(libGLESv2 PRIVATE angle::common angle::libANGLE)
target_compile_definitions(libGLESv2
    PRIVATE -DLIBGLESV2_IMPLEMENTATION
    PUBLIC -DGL_GLEXT_PROTOTYPES
           -DGL_GLES_PROTOTYPES=1
           -DGL_APICALL=
           -DGL_API=
)


##########
# libEGL
file(GLOB LIBEGL_SOURCES "src/libEGL/*.cpp" "src/libEGL/*.h" "src/libEGL/libEGL.def")
add_library(libEGL STATIC ${LIBEGL_SOURCES})
target_link_libraries(libEGL PRIVATE angle::common angle::libANGLE libGLESv2)
target_compile_definitions(libEGL
    PRIVATE -DLIBEGL_IMPLEMENTATION
    PUBLIC  -DEGL_EGL_PROTOTYPES=1
            -DEGL_EGLEXT_PROTOTYPES
            -DEGLAPI=

)


if (APPLE)
    set(OSX_FRAMEWORKS "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation -framework IOSurface -framework QuartzCore -framework CoreGraphics")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OSX_FRAMEWORKS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OSX_FRAMEWORKS}")
endif()

##########
# easy tests
set(EASY_TESTS_SOURCES
    "src/tests/easy/native_smoke_test.cpp"
)
add_executable(easy_tests ${EASY_TESTS_SOURCES})
target_link_libraries(easy_tests PRIVATE GTest::GTest GTest::Main libEGL libGLESv2 ${EXTRA_LIBS})
add_test(AllTestsInMain easy_tests)



##########
# com.android.opengl bindings

if(MSVC)
    set(ANDROID_OPENGL_IGNORE_WARNINGS "/wd4068" "/wd4101" "/wd4244" "/wd4312")
elseif(APPLE)
    set(ANDROID_OPENGL_IGNORE_WARNINGS "-Wno-writable-strings")
elseif(LINUX)
    set(ANDROID_OPENGL_IGNORE_WARNINGS "-Wno-write-strings")
else()
    set(ANDROID_OPENGL_IGNORE_WARNINGS "")
endif()

set(ANDROID_OPENGL_NATIVE_SOURCES
  "src/android.opengl/android.opengl/C/android_opengl_EGL14.cpp"
  "src/android.opengl/android.opengl/C/android_opengl_EGL15.cpp"
  "src/android.opengl/android.opengl/C/android_opengl_GLES20.cpp"
  "src/android.opengl/android.opengl/C/android_opengl_GLES30.cpp"
  "src/android.opengl/android.opengl/C/android_opengl_GLES31.cpp"
  "src/android.opengl/android.opengl/C/android_opengl_GLES32.cpp"
  "src/android.opengl/native/EntryPoints.cpp"
  "src/android.opengl/native/JNIHelp.cpp"
)
add_library(android.opengl.native SHARED ${ANDROID_OPENGL_NATIVE_SOURCES})
target_include_directories(android.opengl.native PRIVATE src/android.opengl ${JNI_INCLUDE_DIRS})
target_compile_options(android.opengl.native PUBLIC ${ANDROID_OPENGL_IGNORE_WARNINGS})
target_link_libraries(android.opengl.native PRIVATE libEGL libGLESv2 ${JNI_LIBRARIES} ${CMAKE_DL_LIBS} ${EXTRA_LIBS})

set(ANDROID_OPENGL_SOURCES
    "src/android.opengl/android.opengl/android/opengl/EGL14.java"
    "src/android.opengl/android.opengl/android/opengl/EGL15.java"
    "src/android.opengl/android.opengl/android/opengl/EGLConfig.java"
    "src/android.opengl/android.opengl/android/opengl/EGLContext.java"
    "src/android.opengl/android.opengl/android/opengl/EGLDisplay.java"
    "src/android.opengl/android.opengl/android/opengl/EGLImage.java"
    "src/android.opengl/android.opengl/android/opengl/EGLObjectHandle.java"
    "src/android.opengl/android.opengl/android/opengl/EGLSurface.java"
    "src/android.opengl/android.opengl/android/opengl/EGLSync.java"
    "src/android.opengl/android.opengl/android/opengl/GLES20.java"
    "src/android.opengl/android.opengl/android/opengl/GLES30.java"
    "src/android.opengl/android.opengl/android/opengl/GLES31.java"
    "src/android.opengl/android.opengl/android/opengl/GLES32.java"  
    "src/android.opengl/android.opengl/android/opengl/JNIHelp.java"
)
add_jar(android.opengl ${ANDROID_OPENGL_SOURCES})


add_jar(java_smoke_test
    "src/tests/easy/JavaSmokeTest.java"
    ENTRY_POINT TestGL
    INCLUDE_JARS android.opengl)
    
add_jar(window_test
    "src/tests/easy/WindowTest.java"
    ENTRY_POINT WindowTest
    INCLUDE_JARS android.opengl)

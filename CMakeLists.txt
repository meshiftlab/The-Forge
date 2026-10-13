cmake_minimum_required(VERSION 3.28)

project(TheForge LANGUAGES C CXX)

set(THE_FORGE_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(THE_FORGE_COMMON_ROOT "${THE_FORGE_ROOT}/Common_3")

if(NOT EXISTS "${THE_FORGE_COMMON_ROOT}/Application/Config.h")
    message(FATAL_ERROR "The-Forge source tree is missing: ${THE_FORGE_COMMON_ROOT}")
endif()

if(WIN32)
    set(_the_forge_supported_backends D3D12 VULKAN)
elseif(APPLE)
    set(_the_forge_supported_backends METAL)
elseif(ANDROID OR UNIX)
    set(_the_forge_supported_backends VULKAN)
else()
    set(_the_forge_supported_backends)
endif()

if(ANDROID)
    # The-Forge 1.63 selects Vulkan on Android only for ARM64 builds.
    # Keep unsupported ABIs from failing later with an opaque renderer-config error.
    if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        message(FATAL_ERROR
            "The-Forge Android renderer-only integration supports arm64-v8a; "
            "got '${CMAKE_ANDROID_ARCH_ABI}'")
    endif()
endif()

if(DEFINED MESHIFT_RENDER_BACKEND)
    # Meshift owns backend selection when The-Forge is embedded. Force the
    # dependency to follow it even when a build directory is reconfigured.
    set(THE_FORGE_RENDER_BACKEND "${MESHIFT_RENDER_BACKEND}")
elseif(NOT DEFINED THE_FORGE_RENDER_BACKEND)
    if(NOT _the_forge_supported_backends)
        message(FATAL_ERROR "The-Forge renderer-only CMake integration does not support this platform")
    endif()
    list(GET _the_forge_supported_backends 0 THE_FORGE_RENDER_BACKEND)
endif()
set(THE_FORGE_RENDER_BACKEND "${THE_FORGE_RENDER_BACKEND}" CACHE STRING "The-Forge renderer backend")
set_property(CACHE THE_FORGE_RENDER_BACKEND PROPERTY STRINGS ${_the_forge_supported_backends})

option(THE_FORGE_BUILD_RESOURCE_LOADER "Build The-Forge's optional resource loader" OFF)
option(THE_FORGE_ENABLE_AMDAGS "Enable AMD AGS integration when its bundled library is available" ON)
option(THE_FORGE_ENABLE_NVAPI "Enable NVIDIA NVAPI integration when its bundled library is available" ON)
option(THE_FORGE_ENABLE_ANDROID_SWAPPY "Enable Android Frame Pacing Library (Swappy) when the bundled arm64 library is available" ON)

if(NOT THE_FORGE_RENDER_BACKEND IN_LIST _the_forge_supported_backends)
    message(FATAL_ERROR "Invalid THE_FORGE_RENDER_BACKEND='${THE_FORGE_RENDER_BACKEND}'")
endif()

if(THE_FORGE_RENDER_BACKEND STREQUAL "D3D12" AND NOT WIN32)
    message(FATAL_ERROR "The-Forge D3D12 backend is only available on Windows")
elseif(THE_FORGE_RENDER_BACKEND STREQUAL "METAL" AND NOT APPLE)
    message(FATAL_ERROR "The-Forge Metal backend is only available on Apple platforms")
elseif(THE_FORGE_RENDER_BACKEND STREQUAL "VULKAN" AND APPLE)
    message(FATAL_ERROR "The-Forge selects Metal on Apple platforms; use THE_FORGE_RENDER_BACKEND=METAL")
elseif(THE_FORGE_RENDER_BACKEND STREQUAL "VULKAN" AND NOT (WIN32 OR UNIX OR ANDROID))
    message(FATAL_ERROR "The-Forge Vulkan backend is unavailable on this platform")
endif()

set(_the_forge_sources
    "${THE_FORGE_COMMON_ROOT}/Graphics/GraphicsConfig.cpp"

    "${THE_FORGE_COMMON_ROOT}/OS/CPUConfig.cpp"

    "${THE_FORGE_COMMON_ROOT}/Utilities/FileSystem/FileSystem.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/Log/Log.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/Math/Algorithms.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/Math/StbDs.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/MemoryTracking/MemoryTracking.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/bstrlib/bstrlib.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/lz4/lz4.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/debug.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/entropy_common.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/error_private.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/fse_decompress.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/pool.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/threading.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/xxhash.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/common/zstd_common.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/decompress/huf_decompress.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_ddict.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress_block.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/Threading/ThreadSystem.c"
    "${THE_FORGE_COMMON_ROOT}/Utilities/Timer.c"

    "${CMAKE_CURRENT_LIST_DIR}/Integration/RendererOnlyPlatform.cpp"
)

if(WIN32)
    list(APPEND _the_forge_sources
        "${THE_FORGE_COMMON_ROOT}/OS/Windows/WindowsFileSystem.cpp"
        "${THE_FORGE_COMMON_ROOT}/OS/Windows/WindowsLog.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Windows/WindowsThread.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Windows/WindowsTime.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_windows.c"
    )
elseif(ANDROID)
    list(APPEND _the_forge_sources
        "${THE_FORGE_COMMON_ROOT}/OS/Android/AndroidFileSystem.cpp"
        "${THE_FORGE_COMMON_ROOT}/OS/Android/AndroidLog.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Android/AndroidThread.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Android/AndroidTime.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/hwcaps.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/impl_aarch64_linux_or_android.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_linux_or_android.c"
        "${THE_FORGE_COMMON_ROOT}/Utilities/FileSystem/UnixFileSystem.c"
    )
elseif(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
    list(APPEND _the_forge_sources
        "${THE_FORGE_COMMON_ROOT}/OS/Darwin/CocoaFileSystem.mm"
        "${THE_FORGE_COMMON_ROOT}/OS/Darwin/DarwinLog.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Darwin/DarwinThread.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/impl_aarch64_iOS.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_macos.c"
        "${THE_FORGE_COMMON_ROOT}/Utilities/FileSystem/UnixFileSystem.c"
    )
elseif(UNIX)
    list(APPEND _the_forge_sources
        "${THE_FORGE_COMMON_ROOT}/OS/Linux/LinuxFileSystem.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Linux/LinuxLog.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Linux/LinuxThread.c"
        "${THE_FORGE_COMMON_ROOT}/OS/Linux/LinuxTime.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/hwcaps.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/impl_aarch64_linux_or_android.c"
        "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_linux_or_android.c"
        "${THE_FORGE_COMMON_ROOT}/Utilities/FileSystem/UnixFileSystem.c"
    )
else()
    message(FATAL_ERROR "The-Forge renderer-only CMake integration does not support this platform")
endif()

if(THE_FORGE_RENDER_BACKEND STREQUAL "VULKAN")
    list(APPEND _the_forge_sources
        "${THE_FORGE_COMMON_ROOT}/Graphics/Vulkan/Vulkan.c"
        "${THE_FORGE_COMMON_ROOT}/Graphics/Vulkan/Vulkan_cxx.cpp"
        "${THE_FORGE_COMMON_ROOT}/Graphics/Vulkan/VulkanRaytracing.c"
    )
elseif(THE_FORGE_RENDER_BACKEND STREQUAL "D3D12")
    list(APPEND _the_forge_sources
        "${THE_FORGE_COMMON_ROOT}/Graphics/Direct3D12/Direct3D12.c"
        "${THE_FORGE_COMMON_ROOT}/Graphics/Direct3D12/Direct3D12_cxx.cpp"
        "${THE_FORGE_COMMON_ROOT}/Graphics/Direct3D12/Direct3D12Hooks.c"
        "${THE_FORGE_COMMON_ROOT}/Graphics/Direct3D12/Direct3D12Raytracing.c"
    )
elseif(THE_FORGE_RENDER_BACKEND STREQUAL "METAL")
    list(APPEND _the_forge_sources
        "${THE_FORGE_COMMON_ROOT}/Graphics/Metal/MetalRenderer.mm"
        "${THE_FORGE_COMMON_ROOT}/Graphics/Metal/MetalRaytracing.mm"
    )
endif()

if(THE_FORGE_BUILD_RESOURCE_LOADER)
    list(APPEND _the_forge_sources "${THE_FORGE_COMMON_ROOT}/Resources/ResourceLoader/ResourceLoader.cpp")

    if(WIN32 OR APPLE OR (UNIX AND NOT ANDROID))
        list(APPEND _the_forge_sources "${THE_FORGE_COMMON_ROOT}/Utilities/FileSystem/ToolFileSystem.c")
    endif()

    if(WIN32)
        list(APPEND _the_forge_sources
            "${THE_FORGE_COMMON_ROOT}/OS/Windows/WindowsToolsFileSystem.cpp"
        )
    elseif(ANDROID)
        # AndroidFileSystem.cpp provides APK and external-storage streams.
        # The upstream Android renderer target intentionally omits the desktop
        # tool filesystem and its directory-iteration API.
    elseif(APPLE)
        if(CMAKE_SYSTEM_NAME MATCHES "iOS|tvOS|watchOS")
            list(APPEND _the_forge_sources
                "${THE_FORGE_COMMON_ROOT}/OS/Darwin/iOSToolsFileSystem.mm"
            )
        else()
            list(APPEND _the_forge_sources
                "${THE_FORGE_COMMON_ROOT}/OS/Darwin/CocoaToolsFileSystem.mm"
            )
        endif()
    elseif(UNIX)
        list(APPEND _the_forge_sources
            "${THE_FORGE_COMMON_ROOT}/OS/Linux/LinuxToolsFileSystem.c"
        )
    endif()
endif()

add_library(TheForge STATIC ${_the_forge_sources})
add_library(TheForge::TheForge ALIAS TheForge)

set_target_properties(TheForge PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(TheForge
    PUBLIC
        "${THE_FORGE_COMMON_ROOT}"
)

if(ANDROID)
    # The renderer-only build keeps Android's public WindowHandle types, but
    # does not compile The-Forge's NativeActivity glue implementation. Expose
    # the NDK header directory so consumers can include the public interfaces.
    set(_the_forge_android_ndk_roots)
    if(CMAKE_ANDROID_NDK)
        list(APPEND _the_forge_android_ndk_roots "${CMAKE_ANDROID_NDK}")
    endif()
    if(ANDROID_NDK)
        list(APPEND _the_forge_android_ndk_roots "${ANDROID_NDK}")
    endif()
    if(DEFINED ENV{ANDROID_NDK_ROOT} AND NOT "$ENV{ANDROID_NDK_ROOT}" STREQUAL "")
        list(APPEND _the_forge_android_ndk_roots "$ENV{ANDROID_NDK_ROOT}")
    endif()
    if(DEFINED ENV{ANDROID_NDK_HOME} AND NOT "$ENV{ANDROID_NDK_HOME}" STREQUAL "")
        list(APPEND _the_forge_android_ndk_roots "$ENV{ANDROID_NDK_HOME}")
    endif()

    set(_the_forge_android_native_app_glue_include "")
    foreach(_the_forge_android_ndk_root IN LISTS _the_forge_android_ndk_roots)
        if(EXISTS "${_the_forge_android_ndk_root}/sources/android/native_app_glue/android_native_app_glue.h")
            set(_the_forge_android_native_app_glue_include
                "${_the_forge_android_ndk_root}/sources/android/native_app_glue")
            break()
        endif()
    endforeach()

    if(_the_forge_android_native_app_glue_include STREQUAL "")
        message(FATAL_ERROR
            "The-Forge Android renderer-only integration requires the NDK "
            "native_app_glue header (android_native_app_glue.h)")
    endif()

    target_include_directories(TheForge
        PUBLIC
            "${_the_forge_android_native_app_glue_include}"
    )
endif()

if(THE_FORGE_RENDER_BACKEND STREQUAL "VULKAN")
    target_include_directories(TheForge
        PUBLIC
            "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/VulkanSDK/include"
    )
endif()

target_compile_definitions(TheForge
    PUBLIC
        $<$<STREQUAL:${THE_FORGE_RENDER_BACKEND},VULKAN>:FORGE_EXPLICIT_RENDERER_API>
        $<$<STREQUAL:${THE_FORGE_RENDER_BACKEND},VULKAN>:FORGE_EXPLICIT_RENDERER_API_VULKAN>
        THE_FORGE_EMBEDDED_RENDERER
        THE_FORGE_DISABLE_RELOAD_SHADER
)

if(WIN32)
    target_compile_definitions(TheForge PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _WINDOWS)
    target_link_libraries(TheForge PRIVATE
        d3d12
        dxgi
        dxguid
        user32
        gdi32
        ole32
        shell32
        shcore
        winmm
    )

    set(_the_forge_dxc_lib
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/lib/x64/dxcompiler.lib")
    set(_the_forge_dxc_dll
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/bin/x64/dxcompiler.dll")
    set(_the_forge_dxil_dll
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/bin/x64/dxil.dll")
    set(_the_forge_pix_lib
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.lib")
    set(_the_forge_pix_dll
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.dll")
    set(_the_forge_ags_lib
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.lib")
    set(_the_forge_ags_dll
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.dll")
    set(_the_forge_nvapi_lib
        "${THE_FORGE_COMMON_ROOT}/Graphics/ThirdParty/OpenSource/nvapi/amd64/nvapi64.lib")

    set(_the_forge_runtime_files)

    if(THE_FORGE_RENDER_BACKEND STREQUAL "D3D12")
        if(EXISTS "${_the_forge_dxc_lib}")
            target_link_libraries(TheForge PRIVATE "${_the_forge_dxc_lib}")
            if(EXISTS "${_the_forge_dxc_dll}")
                list(APPEND _the_forge_runtime_files "${_the_forge_dxc_dll}")
            endif()
            if(EXISTS "${_the_forge_dxil_dll}")
                list(APPEND _the_forge_runtime_files "${_the_forge_dxil_dll}")
            endif()
        else()
            message(WARNING "The-Forge D3D12 backend was selected but DXC was not found: ${_the_forge_dxc_lib}")
        endif()

        if(EXISTS "${_the_forge_pix_lib}")
            target_link_libraries(TheForge PRIVATE "${_the_forge_pix_lib}")
            if(EXISTS "${_the_forge_pix_dll}")
                list(APPEND _the_forge_runtime_files "${_the_forge_pix_dll}")
            endif()
        endif()
    endif()

    if(THE_FORGE_ENABLE_AMDAGS AND EXISTS "${_the_forge_ags_lib}")
        target_link_libraries(TheForge PRIVATE "${_the_forge_ags_lib}")
        if(EXISTS "${_the_forge_ags_dll}")
            list(APPEND _the_forge_runtime_files "${_the_forge_ags_dll}")
        endif()
    else()
        target_compile_definitions(TheForge PRIVATE THE_FORGE_DISABLE_AMDAGS)
    endif()

    if(THE_FORGE_ENABLE_NVAPI AND EXISTS "${_the_forge_nvapi_lib}")
        target_link_libraries(TheForge PRIVATE "${_the_forge_nvapi_lib}")
    else()
        target_compile_definitions(TheForge PRIVATE THE_FORGE_DISABLE_NVAPI)
    endif()
elseif(ANDROID)
    find_package(Threads REQUIRED)
    target_link_libraries(TheForge PRIVATE Threads::Threads ${CMAKE_DL_LIBS} android log)

    set(_the_forge_agdk_root "${THE_FORGE_COMMON_ROOT}/OS/ThirdParty/OpenSource/agdk")
    set(_the_forge_swappy_lib
        "${_the_forge_agdk_root}/libs/arm64-v8a_cpp_static_Release/libswappy_static.a")
    if(THE_FORGE_RENDER_BACKEND STREQUAL "VULKAN")
        target_include_directories(TheForge PRIVATE "${_the_forge_agdk_root}/include")
        set(_the_forge_enable_swappy FALSE)
        set(_the_forge_memory_advice_lib
            "${_the_forge_agdk_root}/libs/arm64-v8a_cpp_static_Release/libmemory_advice_static.a")
        set(_the_forge_paddleboat_lib
            "${_the_forge_agdk_root}/libs/arm64-v8a_cpp_static_Release/libpaddleboat_static.a")
        if(THE_FORGE_ENABLE_ANDROID_SWAPPY
           AND CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a"
           AND EXISTS "${_the_forge_swappy_lib}"
           AND EXISTS "${_the_forge_memory_advice_lib}"
           AND EXISTS "${_the_forge_paddleboat_lib}")
            set(_the_forge_enable_swappy TRUE)
            target_link_libraries(TheForge PRIVATE
                "${_the_forge_swappy_lib}"
                "${_the_forge_memory_advice_lib}"
                "${_the_forge_paddleboat_lib}"
            )
        else()
            target_compile_definitions(TheForge PUBLIC THE_FORGE_DISABLE_SWAPPY)
        endif()
        if(NOT _the_forge_enable_swappy)
            message(STATUS "The-Forge Android Swappy: disabled")
        endif()
    endif()
elseif(UNIX)
    find_package(Threads REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(TheForge PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
    # IOperatingSystem.h exposes Xlib-backed WindowHandle types on Linux.
    # Keep the include path and platform dependency transitive for Meshift's
    # renderer-facing API consumers.
    target_link_libraries(TheForge PUBLIC X11::X11)

    # The Forge's Linux Vulkan headers expose both Xlib and Wayland WSI types.
    # Keep Wayland available when its development headers exist, but do not
    # make an X11-only Linux environment install an unrelated WSI dependency.
    find_path(_the_forge_wayland_include_dir wayland-client.h)
    if(_the_forge_wayland_include_dir)
        target_include_directories(TheForge PUBLIC "${_the_forge_wayland_include_dir}")
    else()
        target_compile_definitions(TheForge PUBLIC THE_FORGE_DISABLE_WAYLAND)
        message(STATUS "The-Forge Linux Wayland WSI: disabled (wayland-client.h not found)")
    endif()
endif()

if(_the_forge_runtime_files)
    set(_the_forge_runtime_copy_commands)
    foreach(_the_forge_runtime_file IN LISTS _the_forge_runtime_files)
        list(APPEND _the_forge_runtime_copy_commands
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_the_forge_runtime_file}"
                "$<TARGET_FILE_DIR:TheForge>"
        )
    endforeach()
    add_custom_command(TARGET TheForge POST_BUILD
        ${_the_forge_runtime_copy_commands}
        COMMENT "Copy The-Forge runtime dependencies"
        VERBATIM
    )
endif()

if(APPLE)
    set(_the_forge_apple_frameworks
        "-framework Foundation"
        "-framework Metal"
        "-framework MetalKit"
        "-framework MetalPerformanceShaders"
        "-framework CoreGraphics"
        "-framework CoreVideo"
        "-framework QuartzCore"
    )
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(APPEND _the_forge_apple_frameworks
            "-framework AppKit"
            "-framework Carbon"
            "-framework CoreServices"
            "-framework IOKit"
        )
    else()
        list(APPEND _the_forge_apple_frameworks "-framework UIKit")
    endif()
    target_link_libraries(TheForge PRIVATE ${_the_forge_apple_frameworks})
endif()

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${_the_forge_sources})
set_target_properties(TheForge PROPERTIES FOLDER "ThirdParty/The-Forge")

message(STATUS "The-Forge source root: ${THE_FORGE_ROOT}")
message(STATUS "The-Forge backend: ${THE_FORGE_RENDER_BACKEND}")

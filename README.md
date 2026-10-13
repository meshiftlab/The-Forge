# The-Forge renderer sources used by Meshift

This directory contains the cross-platform renderer sources from The-Forge
1.63 that are embedded into Meshift.

Meshift builds the renderer through the parent CMake project. The supported
backends are selected with `MESHIFT_RENDER_BACKEND` or
`THE_FORGE_RENDER_BACKEND`:

- Windows: Vulkan or D3D12
- Linux: Vulkan
- Android: Vulkan
- Apple platforms: Metal

The parent project creates the `TheForge::TheForge` target. The optional
resource loader can be enabled with `THE_FORGE_BUILD_RESOURCE_LOADER`.

The original examples, asset/shader tools, and platform application layer are
not part of this embedded renderer build. The renderer sources, platform
support needed by the renderer, Vulkan loader, memory allocators, and backend
headers remain available under `Common_3`.

The upstream license is included in `LICENSE`.

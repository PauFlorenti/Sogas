# ROADMAP
## SUMARY
---
This is the roadmap for the Sogas Engine project.

Not in priority order.

## ITEMS
---

### ORGANIZATION
- [ ] Organize project into private includes and private sources.


### ARCHITECTURE
- [-] Logger
    - [x] Print different hazard levels: Verbose, Trace, Info, Warning, Error, Fatal
    - [x] Print each different level in a color: Grey, Blue, Green, Yellow, Red, Red
    - [ ] Use logger to handle errors and fatals
    - [x] Create own assert function.
    - [ ] Assertion messages should accept variables to the message.

- [ ] Support multiple OS. Maybe own code instead of GLFW
    - [ ] Windows
    - [ ] Linux
    - [ ] MacOS

- [ ] Handle Input
    - [ ] Create Input module
    - [ ] Keyboard
    - [ ] Mouse
    - [ ] Remote controller

- [ ] Read external data (Meshes, textures, json files ...) from inside build or create file path finder.
Finder should be handled by resouce manager.
- [ ] Handle resource files with a Resource interface and a factory methode.
    - [ ] Load meshes
        - [-] .obj files
        - [ ] .glft and .glb files
        - [ ] .fbx files
    - [ ] Load textures

- [ ] Entity Component System
    - [x] Entity class
    - [ ] Transform component - GLM or own math class??
    - [ ] Camera component.
    - [ ] Render component.
        - [ ] Material definition.
    - [ ] Light component.
        - [ ] Directional Light
        - [ ] Spot Light
        - [ ] Point Light
    - [ ] Render component with AABB and Sphere bounding.
    - [ ] Proper destruction of unused entities and handles.

### GRAPHICS
- [ ] Render
    - [ ] Create a proper Renderer interface to communicate with multiple devices.
    - [ ] Create a buffer interface.
    - [ ] Create a texture interface.
    - [ ] Create render command queues.

### VULKAN
    - [ ] Properly Initialize Vulkan
        - [ ] Validate physical device given certain criteria.
        - [ ] Validate logical device creation given certain features and extension needed.
        - [ ] Validate Swapchain creation given device properties (validate extent, format ...).
        - [ ] Save PhysicalDevice properties so we do not have to query them every time they are required.
    - [ ] Handle minimazing and resizing window.
    - [ ] Create Forward rendering.
    - [ ] Create Deferred rendering.
    - [ ] Handle Frustum culling.
    - [ ] PBR shading.
    - [ ] Render Interface for different API usage.
    - [ ] Debug rendering. Eg. Cubes, Spheres, lines ...
    - [ ] Rasterized shadows
        - [ ] Cascade Shadow map
        - [ ] Spot Light shadows
        - [ ] Point Light shadows
 
# ROADMAP
## SUMARY
---
This is the roadmap for the Sogas Engine project.

Not in priority order.

## ITEMS
---
- [ ] Logger
    - [ ] Print different hazard levels: Verbose, Trace, Info, Warning, Error, Fatal
    - [ ] Print each different level in a color: Grey, Blue, Green, Yellow, Red, Red

- [ ] Read external data (Meshes, textures, json files ...) from inside build or create file path finder.
- [ ] Handle resource files (Meshes, textures ...) with a Resource interface.

-[ ] Load meshes
    - [ ] .obj files
    - [ ] .glft and .glb files
    - [ ] .fbx files

- [ ] Entity Component System
    - [ ] Entity class
    - [ ] Transform component - GLM or own math class??
    - [ ] Camera component.
    - [ ] Render component.
        - [ ] Material definition.
    - [ ] Light component.
        - [ ] Directional Light
        - [ ] Spot Light
        - [ ] Point Light
    - [ ] Render component with AABB and Sphere bounding. 

- Render
    - [ ] Properly Initialize Vulkan
        - [ ] Validate physical device given certain criteria.
        - [ ] Validate logical device creation given certain features and extension needed.
        - [ ] Validate Swapchain creation given device properties (validate extent, format ...).
    - [ ] Handle minimazing and resizing window.
    - [ ] Create Deferred rendering.
    - [ ] PBR shading.
    - [ ] Render Interface for different API usage.
    - [ ] Debug lines. Eg. Cubes, Spheres, lines ...
    - [ ] Rasterized shadows
        - [ ] Cascade Shadow map
        - [ ] Spot Light shadows
        - [ ] Point Light shadows
 
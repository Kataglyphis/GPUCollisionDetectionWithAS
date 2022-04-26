# GPUCCollisionDetectionWithAs

Collision Detection using Raytracing acceleration structures:

Idea:
For particle systems:
Store the scene geometry (triangles) in the acceleration structure and shot rays proportionally to a objects speed in its moving direction, in order to find intersections.

For rigid objects physics simulation:
Store AABBs in the acceleration structure and shot rays spanning the bounding box,
in order to find intersection candidates.

Links:
Deferred rendering using subpasses: https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/
Frostbite Standard Material Model:
https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf

Particle systems:
https://www.gamasutra.com/view/feature/3157/building_an_advanced_particle_.php


Physics Simulation particle based:
https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-29-real-time-rigid-body-simulation-gpus


Broadphase collision detection
https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-32-broad-phase-collision-detection-cuda



Libs: 
https://github.com/ocornut/imgui
https://github.com/tinyobjloader/tinyobjloader
https://github.com/glfw/glfw
https://github.com/g-truc/glm
https://github.com/nothings/stb

@ECHO ON
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 render_geometry.frag -o spv/render_geometry.frag.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 render_geometry.vert -o spv/render_geometry.vert.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 render_shading.frag -o spv/render_shading.frag.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 render_shading.vert -o spv/render_shading.vert.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 render_particle.frag -o spv/render_particle.frag.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 render_particle.geom -o spv/render_particle.geom.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 render_particle.vert -o spv/render_particle.vert.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 physics_update_model_matrix.comp -o spv/physics_update_model_matrix.comp.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 physics_collision.comp -o spv/physics_collision.comp.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 path_trace.rchit -o spv/path_trace.rchit.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 path_trace.rgen -o spv/path_trace.rgen.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 path_trace.rmiss -o spv/path_trace.rmiss.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 path_trace.transparent.rahit -o spv/path_trace.transparent.rahit.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 path_trace.shadow.rmiss -o spv/path_trace.shadow.rmiss.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 depth_peeling.vert -o spv/depth_peeling.vert.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 depth_peeling.frag -o spv/depth_peeling.frag.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 depth_peeling_fill_and_sort.vert -o spv/depth_peeling_fill_and_sort.vert.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 depth_peeling_fill_and_sort.frag -o spv/depth_peeling_fill_and_sort.frag.spv

C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 global_voxel_grid_gen.comp -o spv/global_voxel_grid_gen.comp.spv
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 voxel_integration.comp -o spv/voxel_integration.comp.spv
PAUSE
@ECHO ON
C:\VulkanSDK\1.3.204.1\Bin\glslc.exe --target-env=vulkan1.3 particle_simulation.comp -o spv\\particle_simulation.comp.spv 
C:\VulkanSDK\1.3.204.1\Bin\glslc.exe --target-env=vulkan1.3 particle_integration.comp -o spv\\particle_integration.comp.spv
PAUSE
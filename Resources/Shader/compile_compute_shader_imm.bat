@ECHO ON
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 particle_simulation.comp -o spv\\particle_simulation.comp.spv 
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 particle_integration.comp -o spv\\particle_integration.comp.spv
PAUSE
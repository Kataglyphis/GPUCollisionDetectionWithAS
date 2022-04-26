@ECHO ON
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 ..\\Resources\\Shader\\particle_simulation.comp -o ..\\Resources\\Shader\\spv\\particle_simulation.comp.spv 
C:\VulkanSDK\1.2.176.1\Bin\glslc.exe --target-env=vulkan1.2 ..\\Resources\\Shader\\particle_integration.comp -o ..\\Resources\\Shader\\spv\\particle_integration.comp.spv
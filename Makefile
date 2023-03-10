CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -Iinclude

VulkanTest: src/hello_vulkan.cpp src/vulkan_triangle.cpp
	g++ $(CFLAGS) -o VulkanTest src/vulkan_triangle.cpp src/hello_vulkan.cpp $(LDFLAGS)

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest

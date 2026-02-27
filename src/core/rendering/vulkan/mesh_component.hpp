#ifndef AETHERENGINE_RENDERING_MESHCOMPONENT
#define AETHERENGINE_RENDERING_MESHCOMPONENT

#include <vector>

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

#include "material.hpp"
#include <core/rendering/objects/vertex.h>

namespace AetherEngine::Rendering {
	struct MeshData {
		std::vector<AetherEngine::Rendering::Objects::Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	class MeshComponent {
	public:
		MeshComponent(
			VkDeviceSize vertexOffset, 
			VkDeviceSize indexOffset,
			uint32_t indexCount,
			std::shared_ptr<Material> material
		);
		~MeshComponent();

		VkDeviceSize getVertexOffset() const { return m_vertexOffset; }
		VkDeviceSize getIndexOffset() const { return m_indexOffset; }
		uint32_t getIndexCount() const { return m_indexCount; }

		std::shared_ptr<Material> getMaterialPtr() const { return m_material_ptr; }

		void updateUniformBuffer(const glm::mat4& viewProjMatrix);
	private:
		std::shared_ptr<Material> m_material_ptr;

		VkDeviceSize m_vertexOffset = 0;
		VkDeviceSize m_indexOffset = 0;
		uint32_t m_indexCount = 0;
	};
}

#endif

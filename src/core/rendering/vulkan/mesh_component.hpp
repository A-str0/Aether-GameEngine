#ifndef AETHERENGINE_RENDERING_MESHCOMPONENT
#define AETHERENGINE_RENDERING_MESHCOMPONENT

#include <vector>

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

#include "material.hpp"
#include <core/rendering/objects/vertex.h>

namespace AetherEngine::Rendering {
	class MeshComponent {
	public:
		MeshComponent(const std::vector<AetherEngine::Rendering::Objects::Vertex> vertices, 
			const std::vector<uint16_t> indices, 
			uint32_t vertexOffset, 
			uint32_t indexOffset,
			std::shared_ptr<Material> material
		);
		~MeshComponent();

		std::vector<AetherEngine::Rendering::Objects::Vertex> getVertices() const { return m_vertices; }
		std::vector<uint16_t> getIndices() const { return m_indices; }
		
		uint32_t getVertexOffset() const { return m_vertexOffset; }
		uint32_t getIndexOffset() const { return m_indexOffset; }

		std::shared_ptr<Material> getMaterialPtr() const { return m_material_ptr; }

		void updateUniformBuffer(const glm::mat4& viewProjMatrix);
	private:
		std::shared_ptr<Material> m_material_ptr;

		const std::vector<AetherEngine::Rendering::Objects::Vertex> m_vertices;
		const std::vector<uint16_t> m_indices;

		uint32_t m_vertexOffset;
		uint32_t m_indexOffset;
	};
}

#endif

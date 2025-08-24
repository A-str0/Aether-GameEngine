#include "../../rendering/objects/vertex.h" // TODO: change
#include "texture_resource.h"

namespace AetherEngine::ResourceManagment::Objects {
    class AE_Object {
    public:
        const std::vector<Rendering::Objects::Vertex> vertices;
        const std::vector<uint32_t> indices;

        TextureResource texture;

        AE_Object(std::vector<Rendering::Objects::Vertex> _vertices, std::vector<uint32_t> _indices) : vertices(_vertices), indices(_indices) {}

        static AE_Object CreateQuad() {
            AE_Object obj{ 
                { // Vertices
                    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}} 
                },
                { // Indices
                    0, 1, 2, 2, 3, 0
                }
            };

            return obj;
        }
    };
}
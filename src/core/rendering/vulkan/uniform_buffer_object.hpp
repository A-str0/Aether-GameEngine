#ifndef AETHERENGINE_RENDERING_UNIFORMBUFFEROBJECT_H
#define AETHERENGINE_RENDERING_UNIFORMBUFFEROBJECT_H

#include <glm/glm.hpp>

namespace AetherEngine {
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}

#endif
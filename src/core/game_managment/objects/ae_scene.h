#ifndef AETHERENGINE_GAMEMANAGMENT_OBJECTS_SCENE_H
#define AETHERENGINE_GAMEMANAGMENT_OBJECTS_SCENE_H

#include "../../resource_managment/objects/ae_object.h" // TODO: change

namespace AetherEngine::GameManagment::Objects {
    class AE_Scene {
    public:
        void addObject(ResourceManagment::Objects::AE_Object obj) {
            m_objects.push_back(obj);
        }
    private:
        std::vector<ResourceManagment::Objects::AE_Object> m_objects;
    };
}

#endif
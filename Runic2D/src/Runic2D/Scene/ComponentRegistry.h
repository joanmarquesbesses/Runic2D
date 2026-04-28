#pragma once
#include "Runic2D/Core/Core.h"
#include "Entity.h"
#include <functional>
#include <vector>
#include <string>

namespace YAML {
    class Emitter;
    class Node;
}

namespace Runic2D {

    struct ComponentDescriptor
    {
        std::string Name;           // "Health Component"
        std::string Category;       // "Gameplay", "AI", "Physics"...

        // Type-Erased functions — no saben res del tipus concret
        std::function<void(Entity)> AddToEntity;
        std::function<bool(Entity)> HasOnEntity;
        std::function<void(Entity)> DrawImGui;    // Dibuixa l'inspector
        std::function<void(Entity)> RemoveFromEntity;

        std::function<void(Entity src, Entity dst)> CopyComponent;
        std::function<void(YAML::Emitter&, Entity)> Serialize;
        std::function<void(YAML::Node&, Entity)> Deserialize;
        bool IsEngineComponent = false;
    };

    class RUNIC_API ComponentRegistry
    {
    public:
        static void Register(ComponentDescriptor&& descriptor);
        static void Clear();
        static void InitEngineComponents(); // <-- CLAU per quan es descarrega la DLL

        static const std::vector<ComponentDescriptor>& GetAll();

        // Per l'inspector: donat una entitat, quins components té?
        static std::vector<const ComponentDescriptor*> GetPresentOn(Entity entity);

    private:
        static std::vector<ComponentDescriptor> s_Descriptors;
    };
}
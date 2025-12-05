#include "R2Dpch.h"
#include "Entity.h"

#include "Component.h"

namespace Runic2D {

    UUID Runic2D::Entity::GetUUID()
    {
        return GetComponent<IDComponent>().ID;
    }

} // namespace Runic2D
#include "R2Dpch.h"
#include "Entity.h"

#include "Component.h"

namespace Runic2D {

    UUID Runic2D::Entity::GetUUID()
    {
        return GetComponent<IDComponent>().ID;
    }

    void Entity::Destroy()
    {
        if (m_Scene)
        {
            m_Scene->SubmitForDestruction(*this);
        }
    }

} // namespace Runic2D
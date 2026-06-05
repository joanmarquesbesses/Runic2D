#include "R2Dpch.h"
#include "Entity.h"

#include "Component.h"
#include "Runic2D/Math/Math.h"

namespace Runic2D {

    UUID Runic2D::Entity::GetUUID()
    {
        return GetComponent<IDComponent>().ID;
    }

    void Entity::SetParent(Entity parent)
    {
        if (*this == parent) return;

        glm::mat4 oldWorldTransform = GetWorldTransform();

        Unparent(false);

        auto& childRC = GetComponent<RelationshipComponent>();
        childRC.Parent = (entt::entity)parent;

        auto& parentRC = parent.GetComponent<RelationshipComponent>();

        if (parentRC.FirstChild == entt::null)
        {
            parentRC.FirstChild = m_EntityHandle;
        }
        else
        {
            entt::entity prevNode = parentRC.FirstChild;
            while (true)
            {
                Entity prevEntity{ prevNode, m_Scene };
                auto& prevRC = prevEntity.GetComponent<RelationshipComponent>();

                if (prevRC.NextSibling == entt::null)
                {
                    prevRC.NextSibling = m_EntityHandle;
                    childRC.PrevSibling = prevNode;
                    break;
                }
                prevNode = prevRC.NextSibling;
            }
        }

        parentRC.ChildrenCount++;

        glm::mat4 parentWorldTransform = parent.GetWorldTransform();
        glm::mat4 newLocalTransform = glm::inverse(parentWorldTransform) * oldWorldTransform;

        auto& tc = GetComponent<TransformComponent>();
        glm::vec3 newTranslation, newRotation, newScale;
        Math::DecomposeTransform(newLocalTransform, newTranslation, newRotation, newScale);

        tc.SetTranslation(newTranslation);
        tc.SetRotation(newRotation);
        tc.SetScale(newScale);
    }

    void Entity::Unparent(bool convertToWorldSpace)
    {
        auto& childRC = GetComponent<RelationshipComponent>();
        if (childRC.Parent == entt::null)
            return;

        glm::mat4 worldTransform = glm::mat4(1.0f);
        if (convertToWorldSpace) {
            worldTransform = GetWorldTransform();
        }

        Entity parent{ childRC.Parent, m_Scene };
        auto& parentRC = parent.GetComponent<RelationshipComponent>();

        if (childRC.PrevSibling != entt::null)
        {
            Entity prevEntity{ childRC.PrevSibling, m_Scene };
            prevEntity.GetComponent<RelationshipComponent>().NextSibling = childRC.NextSibling;
        }

        if (childRC.NextSibling != entt::null)
        {
            Entity nextEntity{ childRC.NextSibling, m_Scene };
            nextEntity.GetComponent<RelationshipComponent>().PrevSibling = childRC.PrevSibling;
        }

        if (parentRC.FirstChild == m_EntityHandle)
        {
            parentRC.FirstChild = childRC.NextSibling;
        }

        parentRC.ChildrenCount--;

        childRC.Parent = entt::null;
        childRC.NextSibling = entt::null;
        childRC.PrevSibling = entt::null;

        if (convertToWorldSpace) {
            auto& tc = GetComponent<TransformComponent>();
            glm::vec3 newTranslation, newRotation, newScale;
            Math::DecomposeTransform(worldTransform, newTranslation, newRotation, newScale);

            tc.SetTranslation(newTranslation);
            tc.SetRotation(newRotation);
            tc.SetScale(newScale);
        }
    }

    void Entity::InvalidateTransform()
    {
        if (HasComponent<TransformComponent>())
            GetComponent<TransformComponent>().SetDirty();

        if (HasComponent<RectTransformComponent>())
            GetComponent<RectTransformComponent>().m_IsDirty = true;
        if (HasComponent<RelationshipComponent>())
        {
            auto& rel = GetComponent<RelationshipComponent>();
            entt::entity childID = rel.FirstChild;
            while (childID != entt::null)
            {
                Entity childEntity{ childID, m_Scene };
                childEntity.InvalidateTransform(); // Recursió neta!

                childID = childEntity.GetComponent<RelationshipComponent>().NextSibling;
            }
        }
    }

    glm::mat4 Entity::GetWorldTransform()
    {
        glm::mat4 transform = GetComponent<TransformComponent>().GetTransform();

        if (HasComponent<RelationshipComponent>())
        {
            entt::entity parentID = GetComponent<RelationshipComponent>().Parent;
            if (parentID != entt::null)
            {
                Entity parent{ parentID, m_Scene };
                transform = parent.GetWorldTransform() * transform;
            }
        }

        return transform;
    }

    void Entity::Destroy()
    {
        if (m_Scene)
        {
            m_Scene->SubmitForDestruction(*this);
        }
    }

} // namespace Runic2D
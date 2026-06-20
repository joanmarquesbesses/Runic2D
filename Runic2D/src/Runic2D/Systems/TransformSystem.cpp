#include "R2Dpch.h"
#include "TransformSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"

#include "Runic2D/Core/JobSystem.h"

namespace Runic2D {
	void Runic2D::TransformSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		R2D_PROFILE_SCOPE("Transform System: OnUpdate");

		// 1. Organize entities by depth level (BFS)
		// This ensures parents are processed before children
		std::vector<std::vector<entt::entity>> levels;
		levels.reserve(4); // Typical hierarchy depth

		// Level 0: Roots (no parent)
		{
			std::vector<entt::entity> roots;
			// Entities with Transform but NO Relationship are roots
			auto transformOnlyView = scene->GetEntityRegistry().view<TransformComponent>(entt::exclude<RelationshipComponent>);
			roots.insert(roots.end(), transformOnlyView.begin(), transformOnlyView.end());

			// Entities with Transform and Relationship with no parent are also roots
			scene->GetEntityRegistry().view<TransformComponent, RelationshipComponent>().each([&](auto entity, auto&, auto& rel)
				{
					if (rel.Parent == entt::null)
						roots.push_back(entity);
				});

			if (roots.empty()) return;
			levels.emplace_back(std::move(roots));
		}

		// Levels 1+: Children
		auto& regisrty = scene->GetEntityRegistry();
		uint32_t currentLevel = 0;
		while (true)
		{
			std::vector<entt::entity> nextLevel;
			for (auto parent : levels[currentLevel])
			{
				if (regisrty.all_of<RelationshipComponent>(parent))
				{
					auto& rel = regisrty.get<RelationshipComponent>(parent);
					entt::entity child = rel.FirstChild;
					while (child != entt::null)
					{
						nextLevel.push_back(child);
						child = regisrty.get<RelationshipComponent>(child).NextSibling;
					}
				}
			}

			if (nextLevel.empty()) break;
			levels.push_back(std::move(nextLevel));
			currentLevel++;
		}

		// 2. Process each level in parallel using JobSystem::Dispatch
		for (auto& levelEntities : levels)
		{
			uint32_t count = (uint32_t)levelEntities.size();
			uint32_t groupSize = 64;

			auto stats = JobSystem::Dispatch(count, groupSize, [&regisrty, &levelEntities](uint32_t start, uint32_t end)
				{
					R2D_PROFILE_SCOPE("Transform Job");

					for (uint32_t i = start; i < end; i++)
					{
						entt::entity entity = levelEntities[i];
						auto& tc = regisrty.get<TransformComponent>(entity);

						if (!tc.GetDirty()) continue;

						// Calculate local transform
						glm::mat4 local = tc.GetTransform();

						// Multiply by parent world transform if exists
						if (regisrty.all_of<RelationshipComponent>(entity))
						{
							entt::entity parent = regisrty.get<RelationshipComponent>(entity).Parent;
							if (parent != entt::null)
							{
								// Parent's WorldTransform is guaranteed to be up-to-date 
								// because it was in a previous level or processed earlier
								tc.SetWorldTransform(regisrty.get<TransformComponent>(parent).GetWorldTransform() * local);
							}
							else
							{
								tc.SetWorldTransform(local);
							}
						}
						else
						{
							tc.SetWorldTransform(local);
						}

						tc.ClearDirty();
					}
				});

			if (stats.GroupsDispatched > 0)
				JobSystem::Wait();
		}
	}
}

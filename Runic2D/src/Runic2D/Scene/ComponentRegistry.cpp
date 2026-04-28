#include "R2Dpch.h"
#include "ComponentRegistry.h"

namespace Runic2D {

	// Inicialització de la variable estàtica
	std::vector<ComponentDescriptor> ComponentRegistry::s_Descriptors;

	void ComponentRegistry::Register(ComponentDescriptor&& descriptor)
	{
		auto it = std::find_if(s_Descriptors.begin(), s_Descriptors.end(),
			[&descriptor](const ComponentDescriptor& d) { return d.Name == descriptor.Name; });

		if (it != s_Descriptors.end())
		{
			R2D_CORE_WARN("ComponentRegistry: '{}' ja registrat, sobreescrivint.", descriptor.Name);
			*it = std::move(descriptor);
			return;
		}
		/*
		R2D_CORE_ASSERT(descriptor.Add, "ComponentDescriptor '{}': falta lambda 'Add'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Has, "ComponentDescriptor '{}': falta lambda 'Has'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Remove, "ComponentDescriptor '{}': falta lambda 'Remove'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.CopyTo, "ComponentDescriptor '{}': falta lambda 'CopyTo'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Serialize, "ComponentDescriptor '{}': falta lambda 'Serialize'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Deserialize, "ComponentDescriptor '{}': falta lambda 'Deserialize'", descriptor.Name);
		*/
		// Afegim el descriptor a la llista (fem servir std::move per eficiència)
		s_Descriptors.push_back(std::move(descriptor));
		R2D_CORE_TRACE("ComponentRegistry: Component registrat '{0}' [{1}]", s_Descriptors.back().Name, s_Descriptors.back().Category);
	}

	void ComponentRegistry::Clear()
	{
		// AQUESTA ÉS LA LÍNIA SALVA-VIDES
		s_Descriptors.clear();
		R2D_CORE_TRACE("ComponentRegistry: S'han netejat tots els components registrats.");
	}

	const std::vector<ComponentDescriptor>& ComponentRegistry::GetAll()
	{
		return s_Descriptors;
	}

	std::vector<const ComponentDescriptor*> ComponentRegistry::GetPresentOn(Entity entity)
	{
		std::vector<const ComponentDescriptor*> presentComponents;

		// Recorrem tots els components registrats
		for (const auto& desc : s_Descriptors)
		{
			// Si l'entitat té aquest component, guardem el punter al descriptor
			if (desc.HasOnEntity(entity))
			{
				presentComponents.push_back(&desc);
			}
		}

		return presentComponents;
	}

}
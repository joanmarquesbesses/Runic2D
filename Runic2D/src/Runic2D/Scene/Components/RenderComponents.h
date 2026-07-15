#pragma once

#include "Runic2D/Core/Core.h"
#include "Runic2D/Core/UUID.h"
#include "Runic2D/Scene/SceneCamera.h"
#include "Runic2D/Renderer/Texture.h"
#include "Runic2D/Renderer/SubTexture2D.h"
#include "Runic2D/Renderer/Font.h"

#include <glm/glm.hpp>

namespace Runic2D {

	struct RUNIC_API SpriteRendererComponent {
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		UUID TextureUUID = 0;
		Ref<Texture2D> Texture;
		Ref<SubTexture2D> SubTexture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};

	struct RUNIC_API CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct RUNIC_API CameraComponent {
		SceneCamera Camera;
		bool Primary = true; //TODO: think about moving to scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct RUNIC_API TextComponent
	{
		enum class Alignment { Left = 0, Center, Right };

	private:
		std::string m_TextString = "";
		mutable float m_CachedWidth = 0.0f;
		mutable bool m_IsDirty = true;

	public:
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		UUID FontUUID = 0;
		Ref<Font> FontAsset = Font::GetDefault();
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
		bool Visible = true;

		Alignment TextAlignment = Alignment::Center;

		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
		TextComponent(const std::string& text) { SetText(text); }

		void SetText(const std::string& text)
		{
			if (m_TextString != text)
			{
				m_TextString = text;
				m_IsDirty = true;
			}
		}

		const std::string& GetText() const { return m_TextString; }

		float GetTextWidth() const
		{
			if (m_IsDirty && FontAsset)
			{
				m_CachedWidth = FontAsset->GetStringWidth(m_TextString, Kerning);
			}
			return m_CachedWidth;
		}

		void MarkDirty() { m_IsDirty = true; }
	};
}
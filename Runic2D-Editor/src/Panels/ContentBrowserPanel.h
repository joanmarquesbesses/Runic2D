#pragma once

#include <filesystem>
#include "Runic2D/Renderer/Texture.h"

namespace Runic2D {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		float& GetThumbnailSize() { return m_ThumbnailSize; }
		float& GetPadding() { return m_Padding; }
	private:
		void RefreshDirectoryEntries();
	private:
		std::filesystem::path m_CurrentDirectory;

		std::vector<std::filesystem::directory_entry> m_DirectoryEntries;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

		float m_ThumbnailSize = 128.0f;
		float m_Padding = 16.0f;
	};

}

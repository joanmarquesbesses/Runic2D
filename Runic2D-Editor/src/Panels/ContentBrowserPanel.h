#pragma once

#include <filesystem>
#include <functional>

#include "Runic2D/Renderer/Texture.h"

struct FileNode
{
	std::filesystem::path Path;
	std::string Name;
	bool IsDirectory;
	std::vector<FileNode> Children; 
	bool IsExpanded = false;        
};

namespace Runic2D {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		float& GetThumbnailSize() { return m_ThumbnailSize; }
		float& GetPadding() { return m_Padding; }

		void SetOnFileOpenCallback(const std::function<void(const std::filesystem::path&)>& callback)
		{
			m_OnFileOpenCallback = callback;
		}

		void ResetToDefault();
	private:
		void RefreshDirectoryEntries();
		void RefreshTree();
		void BuildTreeNodes(const std::filesystem::path& path, std::vector<FileNode>& nodes);
		void DrawTreeRecursive(std::vector<FileNode>& nodes);
	private:
		std::filesystem::path m_CurrentDirectory;

		std::vector<std::filesystem::directory_entry> m_DirectoryEntries;
		std::vector<FileNode> m_TreeNodes;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

		bool m_FirstFrame = true;

		float m_ThumbnailSize = 128.0f;
		float m_Padding = 16.0f;

		std::function<void(const std::filesystem::path&)> m_OnFileOpenCallback;
	};

}

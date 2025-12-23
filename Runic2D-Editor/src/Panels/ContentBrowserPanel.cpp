#include "R2Dpch.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>

#include "Runic2D/Project/Project.h"

namespace Runic2D {

	ContentBrowserPanel::ContentBrowserPanel()
		: m_FirstFrame(true)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/folder.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/document.png");
	}

	void ContentBrowserPanel::ResetToDefault()
	{
		if (Project::GetActive()) {
			m_CurrentDirectory = Project::GetAssetFileSystemPath("");
			RefreshDirectoryEntries();
			RefreshTree();
		}
	}

	void ContentBrowserPanel::RefreshDirectoryEntries()
	{
		if (m_CurrentDirectory.empty() || !std::filesystem::exists(m_CurrentDirectory))
			return;

		m_DirectoryEntries.clear();
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			m_DirectoryEntries.push_back(directoryEntry);
		}
	}

	void ContentBrowserPanel::RefreshTree()
	{
		m_TreeNodes.clear();
		BuildTreeNodes(Project::GetAssetFileSystemPath(""), m_TreeNodes);
	}

	void ContentBrowserPanel::BuildTreeNodes(const std::filesystem::path& path, std::vector<FileNode>& nodes)
	{
		if (path.empty() || !std::filesystem::exists(path) || !std::filesystem::is_directory(path))
			return;

		for (auto& entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory())
				continue; // Només volem carpetes a l'arbre

			FileNode node;
			node.Path = entry.path();
			node.Name = entry.path().filename().string();
			node.IsDirectory = true;
			node.IsExpanded = false;

			BuildTreeNodes(node.Path, node.Children);

			nodes.push_back(node);
		}
	}

	void ContentBrowserPanel::DrawTreeRecursive(std::vector<FileNode>& nodes)
	{
		for (auto& node : nodes)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

			if (m_CurrentDirectory == node.Path)
				flags |= ImGuiTreeNodeFlags_Selected;

			if (node.Children.empty())
				flags |= ImGuiTreeNodeFlags_Leaf;


			bool opened = ImGui::TreeNodeEx(node.Path.string().c_str(), flags, node.Name.c_str());

			if (ImGui::IsItemClicked())
			{
				m_CurrentDirectory = node.Path;
				RefreshDirectoryEntries();
			}

			if (opened)
			{
				if (!node.Children.empty())
					DrawTreeRecursive(node.Children);

				ImGui::TreePop();
			}
		}
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		ImGui::Columns(2);

		if (m_FirstFrame) // Necessitaràs afegir bool m_FirstFrame = true; al header i constructor
		{
			ImGui::SetColumnWidth(0, 200.0f);
			m_FirstFrame = false;
		}

		ImGui::BeginChild("FolderTree");

		std::filesystem::path assetPath = Project::GetAssetFileSystemPath("");

		ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		if (m_CurrentDirectory == assetPath) rootFlags |= ImGuiTreeNodeFlags_Selected;

		bool rootOpen = ImGui::TreeNodeEx("Assets", rootFlags, "Assets");
		if (ImGui::IsItemClicked())
		{
			m_CurrentDirectory = assetPath;
			RefreshDirectoryEntries();
		}

		if (rootOpen)
		{
			DrawTreeRecursive(m_TreeNodes);
			ImGui::TreePop();
		}

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("FileGrid");

		if (m_CurrentDirectory != assetPath)
		{
			if (ImGui::Button("<-"))
			{
				std::filesystem::path parentPath = m_CurrentDirectory.parent_path();
				if (std::filesystem::equivalent(parentPath, assetPath))
				{
					m_CurrentDirectory = assetPath;
				}
				else
				{
					m_CurrentDirectory = parentPath;
				}

				RefreshDirectoryEntries();
			}
			ImGui::Separator();
		}

		float cellSize = m_ThumbnailSize + m_Padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1) columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		int i = 0;
		for (auto& directoryEntry : m_DirectoryEntries)
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, assetPath);
			std::string filenameString = relativePath.filename().string();

			std::string itemPathString = relativePath.string();
			std::replace(itemPathString.begin(), itemPathString.end(), '\\', '/');

			std::string absolutePathString = path.string();
			std::replace(absolutePathString.begin(), absolutePathString.end(), '\\', '/');

			ImGui::PushID(i++);

			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

			ImGui::ImageButton("btn", (ImTextureID)icon->GetRendererID(), { m_ThumbnailSize, m_ThumbnailSize }, { 0, 1 }, { 1, 0 });

			if (ImGui::BeginDragDropSource())
			{
				const char* itemPath = itemPathString.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, itemPathString.size() + 1);
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
					RefreshDirectoryEntries();
					ImGui::PopID();
					break;
				}
				else if (m_OnFileOpenCallback)
				{
					m_OnFileOpenCallback(std::filesystem::path(absolutePathString));
				}
			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::EndChild();

		ImGui::Columns(1);

		ImGui::End();
	}

}
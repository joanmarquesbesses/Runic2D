#include "R2Dpch.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>

namespace Runic2D {

	// Once we have projects, change this
	static const std::filesystem::path s_AssetPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(s_AssetPath), m_FirstFrame(true)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/folder.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/document.png");

		RefreshDirectoryEntries();
		RefreshTree();
	}

	void ContentBrowserPanel::RefreshDirectoryEntries()
	{
		m_DirectoryEntries.clear();
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			m_DirectoryEntries.push_back(directoryEntry);
		}
	}

	void ContentBrowserPanel::RefreshTree()
	{
		m_TreeNodes.clear();
		BuildTreeNodes(s_AssetPath, m_TreeNodes);
	}

	void ContentBrowserPanel::BuildTreeNodes(const std::filesystem::path& path, std::vector<FileNode>& nodes)
	{
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

		ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		if (m_CurrentDirectory == s_AssetPath) rootFlags |= ImGuiTreeNodeFlags_Selected;

		bool rootOpen = ImGui::TreeNodeEx("Assets", rootFlags, "Assets");
		if (ImGui::IsItemClicked())
		{
			m_CurrentDirectory = s_AssetPath;
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

		if (m_CurrentDirectory != std::filesystem::path(s_AssetPath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
				RefreshDirectoryEntries();
			}
			ImGui::Separator();
		}

		float cellSize = m_ThumbnailSize + m_Padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1) columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : m_DirectoryEntries)
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			std::string filenameString = relativePath.filename().string();

			ImGui::PushID(filenameString.c_str());

			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

			ImGui::ImageButton("btn", (ImTextureID)icon->GetRendererID(), { m_ThumbnailSize, m_ThumbnailSize }, { 0, 1 }, { 1, 0 });

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
					RefreshDirectoryEntries();
				}
				else if (m_OnFileOpenCallback)
				{
					m_OnFileOpenCallback(directoryEntry.path());
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
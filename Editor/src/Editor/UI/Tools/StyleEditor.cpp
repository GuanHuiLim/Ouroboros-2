/************************************************************************************//*!
\file          StyleEditorView.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution (100%)
\par           email: junxiang.leong\@digipen.edu
\date          November 12, 2021
\brief		   Displays the UI for editing the style of the editor

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "StyleEditor.h"
#include "WarningMessage.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>

#include <fstream>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <istream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <filesystem>
#include <iostream>

#include <windows.h>
#include <shobjidl_core.h>
//#include <Editor/GUIglobals.h>

ImGuiID StyleEditor::m_styleeditor_popup = 200;
StyleEditor::StyleEditor()
{
	name = "Default";
	LoadStyle();

	(name+'\0').copy(namebuffer, 100);
}
StyleEditor::~StyleEditor()
{

}
void StyleEditor::Show()
{
	MenuBar();
	SaveStylePopUp();

	// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
	// (without a reference style pointer, we will use one compared locally as a reference)
	ImGuiStyle& style = ImGui::GetStyle();
	// Default to using internal storage as reference
	static bool init = true;

	ref_saved_style = style;
	init = false;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

	if (ImGui::ShowStyleSelector("Colors##Selector"))
		ref_saved_style = style;
	ImGui::ShowFontSelector("Fonts##Selector");

	// Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
	if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
		style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
	{ bool border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
	ImGui::SameLine();
	{ bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? 1.0f : 0.0f; } }
	ImGui::SameLine();
	{ bool border = (style.PopupBorderSize > 0.0f);  if (ImGui::Checkbox("PopupBorder", &border)) { style.PopupBorderSize = border ? 1.0f : 0.0f; } }

	// Save/Revert button

	if (ImGui::InputText("FileName", namebuffer, sizeof(namebuffer), ImGuiInputTextFlags_AutoSelectAll))
		name = namebuffer;
	if (ImGui::Button("Save Ref") && !name.empty())
		SaveStyle();
	ImGui::SameLine();
	if (ImGui::Button("Revert Ref"))
		style = ref;
	ImGui::SameLine();


	ImGui::Separator();

	if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Sizes"))
		{
			ImGui::Text("Main");
			ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			int window_menu_button_position = style.WindowMenuButtonPosition + 1;
			if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
				style.WindowMenuButtonPosition = window_menu_button_position - 1;
			ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
			ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			ImGui::SameLine(); 
			ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
			ImGui::SameLine(); 
			ImGui::Text("Safe Area Padding");
			ImGui::SameLine(); 
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Colors"))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export"))
			{
				
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None)) { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
			if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
			if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* colorName = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(colorName))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
				if (memcmp(&style.Colors[i], &ref.Colors[i], sizeof(ImVec4)) != 0)
				{
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref.Colors[i] = style.Colors[i]; }
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref.Colors[i]; }
				}
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(colorName);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Fonts"))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImFontAtlas* atlas = io.Fonts;
			//HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
			ImGui::PushItemWidth(120);
			for (int i = 0; i < atlas->Fonts.Size; i++)
			{
				ImFont* font = atlas->Fonts[i];
				ImGui::PushID(font);
				//NodeFont(font);
				ImGui::PopID();
			}
			if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
			{
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
				ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
				ImGui::TreePop();
			}

			// Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
			// (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
			const float MIN_SCALE = 0.3f;
			const float MAX_SCALE = 2.0f;

			static float window_scale = 1.0f;
			if (ImGui::DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
				ImGui::SetWindowFontScale(window_scale);
			ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Rendering"))
		{
			ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
			ImGui::SameLine();
			//HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");

			ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
			ImGui::SameLine();
			//HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");

			ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
			ImGui::PushItemWidth(100);
			ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
			if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

			// When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
			ImGui::DragFloat("Circle Tessellation Max Error", &style.CircleTessellationMaxError, 0.005f, 0.10f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::IsItemActive())
			{
				ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
				ImGui::BeginTooltip();
				ImGui::TextUnformatted("(R = radius, N = number of segments)");
				ImGui::Spacing();
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				const float min_widget_width = ImGui::CalcTextSize("N: MMM\nR: MMM").x;
				for (int n = 0; n < 8; n++)
				{
					const float RAD_MIN = 5.0f;
					const float RAD_MAX = 70.0f;
					const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);

					ImGui::BeginGroup();

					ImGui::Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad));

					const float canvas_width = std::fmax(min_widget_width, rad * 2.0f);
					const float offset_x = std::floorf(canvas_width * 0.5f);
					const float offset_y = std::floorf(RAD_MAX);

					const ImVec2 p1 = ImGui::GetCursorScreenPos();
					draw_list->AddCircle(ImVec2(p1.x + offset_x, p1.y + offset_y), rad, ImGui::GetColorU32(ImGuiCol_Text));
					ImGui::Dummy(ImVec2(canvas_width, RAD_MAX * 2));

					ImGui::EndGroup();
					ImGui::SameLine();
				}
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			//HelpMarker("When drawing circle primitives with \"num_segments == 0\" tesselation will be calculated automatically.");

			ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
}

void StyleEditor::MenuBar()
{
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
#ifdef EDITOR_PLATFORM_WINDOWS
		if (ImGui::MenuItem("Open File Explorer"))
		{
			std::filesystem::path p;//this folder path
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
										COINIT_DISABLE_OLE1DDE);
			if (SUCCEEDED(hr))
			{
				IFileOpenDialog* pFileOpen;

				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
									  IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

				if (SUCCEEDED(hr))
				{
					// Show the Open dialog box.
					hr = pFileOpen->Show(NULL);
					// Get the file name from the dialog box.
					if (SUCCEEDED(hr))
					{
						IShellItem* pItem;
						hr = pFileOpen->GetResult(&pItem);
						if (SUCCEEDED(hr))
						{
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
							p = pszFilePath;

							pItem->Release();
						}
					}
					pFileOpen->Release();
				}
				CoUninitialize();
			}

			if (p.empty() == false)
			{
				name = (p.parent_path().string() +'/' + p.stem().string()).c_str();
				LoadStyle();
				(name + '\0').copy(namebuffer, 100);
				WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Loaded");
			}
		}
#endif
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
}

void StyleEditor::SaveStyle()
{
	ref = ref_saved_style = ImGui::GetStyle();
	WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Saved Style");

	std::ofstream ofs;
	ofs.open(name+".settings", std::ios::binary);
	if (ofs.is_open() == false)
		return;
	ofs.write(reinterpret_cast<char*>(&ref), sizeof(ref));
	ofs.close();
}
void StyleEditor::LoadStyle()
{
	std::ifstream ifs;
	ifs.open(name + ".settings",std::ios::binary);
	if (ifs.is_open() == false)
		return;
	ifs.read(reinterpret_cast<char*>(&ref), sizeof(ref));
	ifs.close();
	ImGui::GetStyle() = ref;
}

bool StyleEditor::SaveStylePopUp()
{
	if (ImGui::BeginPopupEx(m_styleeditor_popup,
		ImGuiWindowFlags_Modal 
		| ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Text("Do you want to save your changes?");
		if(ImGui::Button("Yes"))
		{
			SaveStyle();
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return true;
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
		{
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return true;
		}
		ImGui::EndPopup();
	}
	return false;
}

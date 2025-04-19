#include "imgui.h"

namespace c {

	inline ImVec4 accent = ImColor(221, 255, 11);

	namespace bg
	{
		inline ImVec4 background = ImColor(16, 16, 16, 255);
		inline ImVec4 roughness = ImColor(255, 255, 255, 15); // JUST CHANGE ALPHA
		inline ImVec4 outline = ImColor(35, 35, 35, 255);

		inline ImVec2 size = ImVec2(730, 430);
		inline float rounding = 12;	
	}

	namespace child
	{
		inline ImVec4 background = ImColor(20, 20, 20, 255);
		inline ImVec4 border = ImColor(26, 26, 26, 255);
		inline ImVec4 lines = ImColor(36, 36, 36, 255);
		inline float rounding = 4;
	}

	namespace checkbox
	{
		inline ImVec4 background = ImColor(26, 26, 26, 255);
		inline ImVec4 outline = ImColor(35, 35, 35, 255);
		inline ImVec4 mark = ImColor(0, 0, 0, 255);
		inline float rounding = 2;
	}

	namespace slider
	{
		inline ImVec4 background = ImColor(26, 26, 26, 255);
		inline float rounding = 2;
	}

	namespace button
	{
		inline ImVec4 background = ImColor(26, 26, 26, 255);
		inline ImVec4 outline = ImColor(36, 36, 36, 255);
		inline float rounding = 2;
	}

	namespace combo
	{
		inline ImVec4 background = ImColor(26, 26, 26, 255);
		inline ImVec4 outline = ImColor(36, 36, 36, 255);
		inline float rounding = 2;
	}

	namespace keybind
	{
		inline ImVec4 background = ImColor(26, 26, 26, 255);
		inline float rounding = 2;
	}

	namespace input
	{
		inline ImVec4 background = ImColor(26, 26, 26, 255);
		inline ImVec4 outline = ImColor(36, 36, 36, 255);
		inline float rounding = 2;
	}

	namespace picker
	{
		inline ImVec4 background = ImColor(26, 26, 26, 255);
		inline float rounding = 4;
	}

	namespace tabs
	{
		inline ImVec4 line = ImColor(36, 36, 36, 255);
	}

	namespace knobs
	{
		inline ImVec4 background = ImColor(32, 32, 34, 255);

	}

	namespace text
	{
		inline ImVec4 text_active = ImColor(255, 255, 255, 255);
		inline ImVec4 text_hov = ImColor(115, 115, 115, 255);
		inline ImVec4 text = ImColor(73, 73, 73, 255);
	}

}
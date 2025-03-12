#include "imgui.h"

namespace c
{
	inline ImVec4 accent_color = ImColor(250, 132, 86);
	inline ImVec4 accent_low_color = ImColor(250, 132, 86,255/2);
	inline ImVec4 accent_low = ImColor(174, 197, 255, 255 / 2);

	inline ImVec4 accent_text_color = ImColor(245, 245, 255);

	inline ImVec4 accent_text_low_color = ImColor(245, 245, 255, 255 / 2);

	inline ImVec4 notify = ImColor(43, 48, 54);


	namespace bg
	{
		inline ImVec4 background = ImColor(14, 14, 15, 255);
		inline ImVec4 outline_background = ImColor(27, 29, 32, 255);
		inline ImVec2 size = ImVec2(750, 640);
		inline float rounding = 12;
	}

	namespace child
	{
		inline ImVec4 background = ImColor(22, 23, 26, 250);
		inline ImVec4 outline_background = ImColor(27, 29, 32, 255);
		inline float rounding = 6;
	}

	 
	namespace checkbox
	{
		inline ImVec4 circle_inactive = ImColor(43, 48, 54, 255);

		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 30;
	}

	namespace slider
	{
		inline ImVec4 circle_inactive = ImColor(43, 48, 54, 255);

		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 30;
	}

	namespace combo
	{
		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 3;
	}

	namespace picker
	{
		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 2;
	}

	namespace button
	{
		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 4;
	}

	namespace input
	{
		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 4;
	}

	namespace keybind
	{
		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 3;
	}


	namespace text
	{
		inline ImVec4 text_hov = ImColor(245, 245, 255);
		inline ImVec4 text = ImColor(90, 93, 100);
		inline ImVec4 text2 = ImColor(90, 93, 100,0);
		inline ImVec4 hide_text = ImColor(43, 48, 54, 255);

	}
}
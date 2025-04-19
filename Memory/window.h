#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <Windows.h>
#include <map>
#include <string>
#include <vector>
#include "xorstr.hpp"
 
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct font_data {
	ImFont* ptr;
	std::string path;
	std::string name;
	float size;
};

namespace window {
	enum class OverlayMode : uint8_t {
		DEFAULT,
		OVERLAY,
		TRANSPARENT_OVERLAY
	};

	inline int wx;
	inline int wy;

	inline int resize_width;
	inline int resize_height;

	// used for file name encryption
	inline uint32_t user_id = 0;

	inline OverlayMode overlay = OverlayMode::DEFAULT;
	inline bool allow_clicks = true;
	inline bool is_setup = false;
	inline float current_hue;
	inline bool vsync = true;

	inline int target_monitor = -1;
	inline int fps_limit = 144;
	inline int icon_index = 0;
	inline HINSTANCE hInstance = NULL;

	inline int monitor_enum_state = 0;

	inline font_data* selected_font = NULL;
	inline font_data* default_font = NULL;
	inline std::vector<font_data*> font_list = {};

	inline bool fonts_need_rebuild = false;

	// Direct3D variables
	inline ID3D11Device* g_pd3dDevice = NULL;
	inline ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
	inline IDXGISwapChain* g_pSwapChain = NULL;
	inline ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
	inline HWND hwnd;

	// ImGui variables
	inline ImGuiIO* g_pIO = NULL;
	inline ImFont* small_font = NULL;

	inline void* render_hook = NULL;
	inline void* styling_hook = NULL;
	inline bool* running = NULL;

	inline void render_frame();
	inline  LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	inline void setup_graphics(HWND hwnd);
	inline void clean_up();
	inline void clean_render_target();
	inline void create_render_target();

	// once running is set to false the window will close
	void init(bool* running);
	bool is_minimized();
	void update_overlay();
	void set_mouse_passthrough(bool state);
	void set_monitor(int index);
	void set_overlay(bool state, bool transparent);

	// fonts
	font_data* load_font(std::string font, int size, bool set_default = false, bool set_selected = true);
	ImFont* get_selected_font();
	font_data* get_font_data(ImFont* imfont);
	font_data* get_font_by_name(std::string name, float size);

	// render functions
	void outlined_text(const ImVec2& pos, ImU32 color, const char* text);
	void push_disabled();
	void pop_disabled();
	void tooltip(const char* tip);
	void kmbox_key_selector(const char* label, int* target, const std::vector<std::pair<std::string, int>>& keys);
}
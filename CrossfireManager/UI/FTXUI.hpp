#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <sstream>
#include <algorithm>
#define NOMINMAX
#include <atomic>
#include <thread>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/box.hpp>
#include <ftxui/util/ref.hpp>
#include "Classes.h"
#include "Memory/Memory.h"

namespace KLASSES {
	extern bool show_addresses;
	extern bool show_entity_list;
	extern bool show_module;
	extern std::atomic<bool> keep_updating;

	void trim(std::string& str);
	ftxui::Element DisplayAddresses(Memory& mem);
	void PrintPlayerInfo(Memory& mem);
	ftxui::Component DisplayStats(ftxui::ScreenInteractive& screen, Memory& mem);
	ftxui::Component GraphThis(ftxui::ScreenInteractive& screen, Memory& mem);
	ftxui::Element DumpModule(ftxui::ScreenInteractive& screen);

	struct PlayerState {
		int selected_player = -1;
		std::map<int, std::vector<float>> player_kd_history;
		std::map<int, std::vector<float>> player_ttk_history;
		std::map<int, std::tuple<int, int, int>> player_details; 
	};
}
 

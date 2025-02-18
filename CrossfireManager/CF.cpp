#define NOMINMAX

#include <windows.h>
#include <atomic>
#include <chrono>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <future>
#include <iostream>
#include <thread>
#include "Memory/Memory.h"
#include "Memory/InputManager.h"
#include "Classes.h"
#include <window.h>
#include <Overlay.h>
#include "ftxui/component/component.hpp"
#include "offsets.h"
#include "FTXUI.hpp"
#include <Triggerbot.h>
#include "imgui.h"
#include <ESPManager.hpp>

std::string dma_status = "Connecting to DMA ...";
std::string update_status = "Updating Offsets";
std::atomic<bool> dma_success(false);

 
 







void InitializeDMA(ftxui::ScreenInteractive* screen) {
	for (int attempts = 0; attempts < 3; ++attempts) {
		if (mem.Init("crossfire.exe",true)) break;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	CFBASE = mem.GetBaseDaddy("crossfire.exe");
	CFSHELL = mem.GetBaseDaddy("CShell_x64.dll");

	dma_status = (CFBASE && CFSHELL) ? "DMA Connected" : "Failed to find Crossfire.exe!";
	dma_success.store(CFBASE && CFSHELL);
	//dma_status += kmBoxBMgr.init() == -1 ? ", but KMBOX Failed!" : " & KMBOX Connected";

	screen->PostEvent(ftxui::Event::Custom);
}

void UpdateOffsets(ftxui::ScreenInteractive* screen) {
	while (!dma_success.load()) std::this_thread::sleep_for(std::chrono::milliseconds(50));

	update_status = KLASSES::Update(mem) ? "Offsets Updated \a \t" : "Failed to Update Offsets!";
	screen->PostEvent(ftxui::Event::Custom);
}

ftxui::Component DisplayStatsAndGraph(ftxui::ScreenInteractive& screen, Memory& mem) {
	 
	if (!KLASSES::show_entity_list) {
		return ftxui::Renderer([] { return ftxui::text("Not in Game") | ftxui::border; });
	}

	static int table_size = 50;
	return ftxui::ResizableSplitLeft(
		ftxui::Container::Vertical({
			KLASSES::DisplayStats(screen, mem) | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50)
			}) | ftxui::bgcolor(ftxui::Color::Black),
		ftxui::Container::Vertical({
			KLASSES::GraphThis(screen, mem) | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50)
			}) | ftxui::bgcolor(ftxui::Color::Black),
		&table_size);
}
static std::jthread espThread;
 
void HandleMenuSelection(int selected, ftxui::ScreenInteractive& screen, ftxui::Component& dynamic_content, std::jthread& triggerbotThread) {

	if (update_status != "Offsets Updated \a \t") {
		dynamic_content = ftxui::Renderer([] { return ftxui::text("Waiting for offsets...") | ftxui::border; });
		screen.PostEvent(ftxui::Event::Custom);
		return;
	}

	switch (selected) {
	case 0: {
		 
		offs::ESPDRAWBOX = !offs::ESPDRAWBOX;
		screen.PostEvent(ftxui::Event::Custom); 
		break;
	}


	case 1:
	{
		static std::jthread glowThread;
		glowThread = std::jthread([&]() {
			KLASSES::LTClientShell CLIENT = mem.Read<KLASSES::LTClientShell>(LT_SHELL);
			for (int i = 0; i < 16; i++) {
				KLASSES::pPlayer player = CLIENT.GetPlayerByIndex(i);
				if (!player.hObject)
					continue;
				//KLASSES::HookGlowESPS(mem, reinterpret_cast<uintptr_t>(player.hObject), true, 255, 0, 0);
			}
			screen.PostEvent(ftxui::Event::Custom);
			});
		break;
	}
	case 2:
	{
		KLASSES::LTClientShell cltShell = mem.Read<KLASSES::LTClientShell>(LT_SHELL);

		if (!cltShell.CPlayerClntBase) {
			dynamic_content =
				ftxui::Renderer([] { return ftxui::text("Not in Game") | ftxui::border; });
			screen.PostEvent(ftxui::Event::Custom);

		}

		KLASSES::show_entity_list = !KLASSES::show_entity_list;
		dynamic_content = DisplayStatsAndGraph(screen, mem);

		screen.PostEvent(ftxui::Event::Custom);
		break;

	}

	case 3:
		KLASSES::show_addresses = !KLASSES::show_addresses;
		dynamic_content = ftxui::Renderer([&] {
			return KLASSES::show_addresses ? KLASSES::DisplayAddresses(mem) : ftxui::emptyElement();
			});
		screen.PostEvent(ftxui::Event::Custom);
		break;

	case 4:
		if (!triggerbotThread.joinable()) {
			triggerbotThread = std::jthread([] {});
			KLASSES::Triggerbot::Triggerbot(mem);
		}
		screen.PostEvent(ftxui::Event::Custom);
		break;

	case 5:
		KLASSES::show_module = !KLASSES::show_module;
		dynamic_content = ftxui::Renderer([&] {
			return KLASSES::show_module ? KLASSES::DumpModule(screen) : ftxui::emptyElement();
			});
	 
		 
		screen.PostEvent(ftxui::Event::Custom);
		break;
	}
}

int main() {

	std::thread overlayThread([] {
		static bool keep_running = true;
		window::init(&keep_running);
		});
	overlayThread.detach();

	 
	auto screen = ftxui::ScreenInteractive::Fullscreen();
	std::jthread initThread([&screen]() {
		InitializeDMA(&screen);
		});
	std::jthread updateThread([&screen]() {
		UpdateOffsets(&screen);
		});

	std::cout << "\033[2J\033[1;1H";
	 
	int selected = 0;
	std::vector<std::string> entries = { "ESP", "Aimbot", "Print EntityList", "Print Address", "TriggerBot", "Offset Finder" };
	auto menu = ftxui::Menu(&entries, &selected);

	ftxui::Component dynamic_content = ftxui::Renderer([] { return ftxui::emptyElement(); });
	std::jthread triggerbotThread;

	menu |= ftxui::CatchEvent([&](ftxui::Event event) {
		// Handle mouse hover tracking
		if (event.is_mouse()) {
			auto& mouse = event.mouse();

			// Update selection on hover
			if (mouse.motion == ftxui::Mouse::Moved) {
				const int new_selected = mouse.y; // Account for header and borders
				if (new_selected >= 0 && new_selected < entries.size()) {
					selected = new_selected;
					return true;
				}
			}

			// Handle left click
			if (mouse.button == ftxui::Mouse::Left &&
				mouse.motion == ftxui::Mouse::Released) {
				HandleMenuSelection(selected, screen, dynamic_content, triggerbotThread);
				return true;
			}
		}

		// Handle keyboard enter
		if (event == ftxui::Event::Return) {
			HandleMenuSelection(selected, screen, dynamic_content, triggerbotThread);
			return true;
		}

		return false;
		});


	auto render_item = [](const std::string& text, bool is_selected, bool is_hovered) {
		auto style = ftxui::text(text);
		if (is_selected) style |= ftxui::inverted;
		if (is_hovered) style |= ftxui::bgcolor(ftxui::Color::GrayDark);
		return style;
		};
	auto styled_menu = menu | ftxui::border | ftxui::bgcolor(ftxui::Color(10, 10, 10)) |
		ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 15) |
		ftxui::size(ftxui::HEIGHT, ftxui::GREATER_THAN, 6) |
		ftxui::flex |
		ftxui::Renderer([](ftxui::Element e) {
		return e | ftxui::vscroll_indicator | ftxui::frame;
			});

	auto container = ftxui::Container::Vertical(
		{ ftxui::Renderer([] {
				return ftxui::vbox({
					ftxui::blink(ftxui::border(ftxui::text("Makimura's Crossfire DMA V1"))) | ftxui::bold
					| ftxui::color(ftxui::Color::White) | ftxui::center,
					ftxui::separator(),
				});
			}),
			styled_menu }) | ftxui::flex;

	auto renderer = ftxui::Renderer(container, [&] {
		return ftxui::vbox({
			container->Render() | ftxui::center | ftxui::flex,
			ftxui::separator(),
			dynamic_content->Render() | ftxui::center | ftxui::flex,
			ftxui::hbox({
				ftxui::text("[ DMA Status: ") | ftxui::color(ftxui::Color::White),
				ftxui::text(dma_status) | ftxui::color(dma_status.contains("Connected") ? ftxui::Color::Green : ftxui::Color(255, 0, 0)),


				// ftxui::color(dma_status.contains(target) ? dma_status.contains(target2) ? ftxui::Color(255, 255, 0) : ftxui::Color::Green : ftxui::Color(255, 0, 0)),  // wrote this  part myself, unncessary but great 
				ftxui::text("  Offsets: ") | ftxui::color(ftxui::Color::White),
				ftxui::text(update_status) | ftxui::color(ftxui::Color(0, 255, 255)),
				ftxui::text(" ]") | ftxui::color(ftxui::Color::White),
			}) | ftxui::center
			}) | ftxui::border | ftxui::bgcolor(ftxui::Color::Black) | ftxui::flex;
		});

	std::cout << "\033[?1049h";
	screen.Loop(renderer);
	std::cout << "\033[?1049l";
	return 0;
}
#include "FTXUI.hpp"
#define NOMINMAX  // Prevents Windows.h from defining min/max macros
#include <windows.h>
using namespace KLASSES;

bool KLASSES::show_addresses = false;
bool KLASSES::show_entity_list = false;
bool KLASSES::show_module = false;
std::atomic<bool> KLASSES::keep_updating = true;

void KLASSES::trim(std::string& str) {
    str.erase(str.find_last_not_of(' ') + 1);
    str.erase(0, str.find_first_not_of(' '));
}

ftxui::Element KLASSES::DisplayAddresses(Memory& mem) {
    if (!show_addresses) return ftxui::emptyElement();
    LTClientShell shell = mem.Read<LTClientShell>(LT_SHELL);
    auto to_hex = [](uintptr_t ptr) {
        std::stringstream ss;
        ss << "0x" << std::hex << ptr;
        return ss.str();
        };
    std::vector<std::vector<ftxui::Element>> address_data = {
        {ftxui::text("Address Name") | ftxui::bold, ftxui::text("Hex Value") | ftxui::bold},
        {ftxui::text("CFBASE :"),      ftxui::text(to_hex(CFBASE))},
        {ftxui::text("CFSHELL :"),     ftxui::text(to_hex(CFSHELL))},
        {ftxui::text("LTClientShell :"), ftxui::text(to_hex(LT_SHELL))},
        {ftxui::text("Ccamera :"),    ftxui::text(to_hex(reinterpret_cast<uintptr_t>(shell.CCamera)))},
        {ftxui::text("LTClient :"),   ftxui::text(to_hex(reinterpret_cast<uintptr_t>(shell.CLTClient)))},
        {ftxui::text("CGameui :"),    ftxui::text(to_hex(reinterpret_cast<uintptr_t>(shell.CGameUI)))},
        {ftxui::text("ENT_BEGIN "),   ftxui::text(to_hex(offs::dwCPlayerStart))},
        {ftxui::text("ENT_SIZE"),     ftxui::text(to_hex(offs::dwCPlayerSize))},
        {ftxui::text("NULL"),         ftxui::text(to_hex(offs::dwCPlayerSize))}
    };
    ftxui::Table table = ftxui::Table(address_data);
    table.SelectRow(0).Decorate(ftxui::bold);
    table.SelectAll().Border(ftxui::LIGHT);
    table.SelectRow(0).Border(ftxui::DOUBLE);
    auto content = table.SelectRows(1, -1);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Blue), 3, 0);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Cyan), 3, 1);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::White), 3, 2);
    return ftxui::vbox({
        ftxui::text(" Address List ") | ftxui::bold | ftxui::center,
        ftxui::separator(),
        table.Render()
        }) | ftxui::border | ftxui::color(ftxui::Color::White);
}

void KLASSES::PrintPlayerInfo(Memory& mem) {
    constexpr uintptr_t START_OFFSET = 0x270;
    constexpr uintptr_t END_OFFSET = 0x300;
    constexpr int MAX_PLAYERS = 24;
    LTClientShell cltShell = mem.Read<LTClientShell>(LT_SHELL);
    for (uintptr_t offset = START_OFFSET; offset <= END_OFFSET; offset += sizeof(int)) {
        uintptr_t testAddr = LT_SHELL + offset;
        int candidateIndex = mem.Read<int>(testAddr);
        if (candidateIndex < 0 || candidateIndex >= MAX_PLAYERS) continue;
        pPlayer player = cltShell.GetPlayerByIndex(candidateIndex);
        if (!player.hObject) continue;
        std::string playerName;
        for (char c : player.Name) playerName += (std::isprint(c) && !std::isspace(c)) ? c : ' ';
        playerName = playerName.substr(0, playerName.find('\0'));
        playerName.erase(std::remove(playerName.begin(), playerName.end(), ' '), playerName.end());
        if (playerName == "Jorny") {
            Utils::DebugLog("[FOUND] Your index %d is stored at LT_SHELL + 0x%llX", candidateIndex, offset);
            return;
        }
    }
    Utils::DebugLog("Local player index not found in scanned range");
}

 
ftxui::Component KLASSES::DisplayStats(ftxui::ScreenInteractive& screen, Memory& mem) {
    return ftxui::Renderer([&] {
        if (!KLASSES::show_entity_list) return ftxui::emptyElement();

        LTClientShell cltShell = mem.Read<LTClientShell>(LT_SHELL);
        if (LT_SHELL == 0)
            return ftxui::text("Invalid LT_SHELL address") | ftxui::border;

        const int MAX_PLAYERS = 24;

        auto to_string = [](int value) -> std::string {
            return std::to_string(value);
            };


        std::vector<std::vector<ftxui::Element>> player_data = {
            {ftxui::text("Name   ") | ftxui::bold, ftxui::text(" Health ") | ftxui::bold,
             ftxui::text("Kills  ") | ftxui::bold,
             ftxui::text("  Deaths  ") | ftxui::bold} };
          
        for (int i = 0; i < MAX_PLAYERS; i++) {
            pPlayer player = cltShell.GetPlayerByIndex(i);
            if (!player.hObject) continue;

            std::string sanitizedName;
            for (char c : player.Name) {
                sanitizedName += std::isprint(c) ? c : ' ';
            }
            sanitizedName = sanitizedName.substr(0, 16);

            if (sanitizedName.empty() ||
                sanitizedName.find_first_not_of(' ') == std::string::npos) {
                continue;
            }

            player_data.push_back({ ftxui::text(sanitizedName),
                                   ftxui::text(to_string(player.Health)) |
                                       ftxui::color(ftxui::Color::Red),
                                   ftxui::text(to_string(player.Kills)) |
                                       ftxui::color(ftxui::Color::Green),
                                   ftxui::text(to_string(player.Deaths)) |
                                       ftxui::color(ftxui::Color::Blue) });
        }

        screen.PostEvent(ftxui::Event::Custom);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (player_data.size() == 1) {
            player_data.push_back({ ftxui::text(" "), ftxui::text("-"),
                                   ftxui::text("-"), ftxui::text("-") });
        }

        player_data.push_back({ ftxui::text(" "), ftxui::text(" "), ftxui::text(" "),
                               ftxui::text(" ") });

        ftxui::Table table = ftxui::Table(player_data);
        table.SelectRow(0).Decorate(ftxui::bold);
        table.SelectAll().Border(ftxui::LIGHT);
        table.SelectRow(0).Border(ftxui::DOUBLE);

        auto content = table.SelectRows(1, -1);
        content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Blue), 3, 0);
        content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Cyan), 3, 1);
        content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::White), 3, 2);

        return ftxui::vbox(
            { ftxui::text(" Player Stats ") | ftxui::bold | ftxui::center,
             ftxui::separator(), table.Render() }) |
            ftxui::border | ftxui::color(ftxui::Color::White);
        });
}







 

ftxui::Element KLASSES::DumpModule(ftxui::ScreenInteractive& screen) {
    // ✅ Fetch all module names from `crossfire.exe`
    std::vector<std::string> modules = mem.GetModuleList("crossfire.exe");

    if (modules.empty()) {
        return ftxui::text("Error: Could not retrieve module list.") | ftxui::border | ftxui::color(ftxui::Color::Red);
    }

     
    std::vector<std::vector<ftxui::Element>> module_data = {
        {ftxui::text("Module Name") | ftxui::bold}
    };

     
    for (const auto& moduleName : modules) {
        module_data.push_back({ ftxui::text(moduleName) | ftxui::color(ftxui::Color::Cyan) });
    }

     
    ftxui::Table table = ftxui::Table(module_data);
    table.SelectRow(0).Decorate(ftxui::bold);
    table.SelectAll().Border(ftxui::LIGHT);
    table.SelectRow(0).Border(ftxui::DOUBLE);

    auto content = table.SelectRows(1, -1);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Blue), 3, 0);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Cyan), 3, 1);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::White), 3, 2);

    return ftxui::vbox({
        ftxui::text(" CFBASE Module List ") | ftxui::bold | ftxui::center,
        ftxui::separator(),
        table.Render()
        }) | ftxui::border | ftxui::color(ftxui::Color::White);
}










ftxui::Component KLASSES::GraphThis(ftxui::ScreenInteractive& screen, Memory& mem) {
    static std::atomic<bool> show_graph = true;
    static std::vector<float> kd_history;
    static std::vector<float> ttk_history;
    static std::mutex data_mutex;
    constexpr int MAX_PLAYERS = 24;
    constexpr int MAX_HISTORY = 50;
    static std::once_flag init_flag;
    std::call_once(init_flag, [&]() {
        std::thread([&]() {
            while (show_graph) {
                if (LT_SHELL == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }
                LTClientShell cltShell = mem.Read<LTClientShell>(LT_SHELL);
                int total_kills = 0, total_deaths = 0;
                float total_time_to_kill = 0;
                int player_count = 0;
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    pPlayer player = cltShell.GetPlayerByIndex(i);
                    if (!player.hObject) continue;
                    total_kills += player.Kills;
                    total_deaths += player.Deaths;
                    total_time_to_kill += (player.Kills > 0) ? (5000.0f / player.Kills) : 0;
                    player_count++;
                }
                float avg_kd = (total_deaths > 0)
                    ? static_cast<float>(total_kills) / total_deaths
                    : static_cast<float>(total_kills);
                float avg_ttk = (player_count > 0)
                    ? total_time_to_kill / player_count : 0.0f;
                {
                    std::lock_guard<std::mutex> lock(data_mutex);
                    kd_history.push_back(avg_kd);
                    ttk_history.push_back(avg_ttk);
                    if (kd_history.size() > MAX_HISTORY) kd_history.erase(kd_history.begin());
                    if (ttk_history.size() > MAX_HISTORY) ttk_history.erase(ttk_history.begin());
                }
                screen.PostEvent(ftxui::Event::Custom);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            }).detach();
        });
    return ftxui::Renderer([&] {
        std::lock_guard<std::mutex> lock(data_mutex);
        float latest_kd = kd_history.empty() ? 0.0f : kd_history.back();
        float latest_ttk = ttk_history.empty() ? 0.0f : ttk_history.back();
        std::stringstream kd_stream, ttk_stream;
        kd_stream << std::fixed << std::setprecision(2) << latest_kd;
        ttk_stream << std::fixed << std::setprecision(2) << latest_ttk;
        auto kd_graph = ftxui::graph([&](int width, int height) {
            std::vector<int> graph_data;
            for (float val : kd_history) {
                graph_data.push_back(static_cast<int>(val * height / 10));
            }
            return graph_data;
            });
        auto ttk_graph = ftxui::graph([&](int width, int height) {
            std::vector<int> graph_data;
            for (float val : ttk_history) {
                graph_data.push_back(static_cast<int>(val * height / 1000));
            }
            return graph_data;
            });
        return ftxui::vbox({
            ftxui::text("Lobby Performance Over Time") | ftxui::bold | ftxui::center,
            ftxui::separator(),
            ftxui::hbox({
                ftxui::vbox({
                    ftxui::text("K/D Ratio") | ftxui::bold | ftxui::center,
                    ftxui::text("Latest: " + kd_stream.str()) | ftxui::color(ftxui::Color::Green) | ftxui::center,
                    kd_graph | ftxui::border
                }) | ftxui::flex,
                ftxui::separator(),
                ftxui::vbox({
                    ftxui::text("Time Between Kills (ms)") | ftxui::bold | ftxui::center,
                    ftxui::text("Latest: " + ttk_stream.str()) | ftxui::color(ftxui::Color::Red) | ftxui::center,
                    ttk_graph | ftxui::border
                }) | ftxui::flex
            }) | ftxui::flex
            }) | ftxui::border
            | ftxui::bgcolor(ftxui::Color(30, 30, 30))
                    | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 80)
                    | ftxui::size(ftxui::HEIGHT, ftxui::GREATER_THAN, 20);
        });
}

 



#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX
#include <Windows.h>
#include "auth.hpp"
#include <string>
#include <thread>
#include <iostream>
#include "skStr.h"
#include "VMProtectSDK.h"
#include <urlmon.h>
#include <regex>
#include <notify.h>
#pragma comment(lib, "urlmon.lib")


using namespace KeyAuth;

std::string g_sanitizedKey;
namespace {
     
    const std::string appName = xorstr_("CrossfireManager");
    const std::string ownerid = xorstr_("6oVNWtNqyj");
    const std::string version = xorstr_("1.0");
    const std::string apiUrl = xorstr_("https://keyauth.win/api/1.3/");
    const std::string path = xorstr_("");

    
    api KeyAuthApp(appName, ownerid, version, apiUrl, path);

    

  std::string sanitizeLicense(const std::string& license) {
        const std::regex validPattern(R"(^CROSSFIRE-[A-Za-z0-9]{6}(?:-[A-Za-z0-9]{6}){3}$)", std::regex::icase);
        return std::regex_match(license, validPattern) ? license : "";
    }

    
    time_t stringToTime(const std::string& str) {
        try { return static_cast<time_t>(std::stoll(str)); }
        catch (...) { return 0; }
    }

    
    std::string timeToReadable(time_t t) {
        tm timeinfo;
        localtime_s(&timeinfo, &t);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return std::string(buffer);
    }
}
 
  void GreetUser() {

    std::string displayKey = g_sanitizedKey;
    size_t lastDash = displayKey.rfind('-');
    if (lastDash != std::string::npos) {
        size_t secondLastDash = displayKey.rfind('-', lastDash - 1);
        if (secondLastDash != std::string::npos) {
            displayKey = displayKey.substr(secondLastDash + 1);
        }
    }


    std::string message = "Welcome Back, " + displayKey;


    ImGui::InsertNotification({ ImGuiToastType_Success, 6000, message.c_str() });
}
void VerifyLogin() {
     
    if (VMProtectIsDebuggerPresent(true)) {
        std::cerr << xorstr_("Unwanted Application Detected.. Exiting..") << std::endl;
        Sleep(2000);
        exit(EXIT_FAILURE);
    }
    if (!VMProtectIsValidImageCRC()) {
        std::cerr << xorstr_("Corrupted Application...Exiting...") << std::endl;
        Sleep(2000);
        exit(EXIT_FAILURE);
    }

     
    KeyAuthApp.init();
    // Optionally: AutoUpdate();

    
    std::string key;
    std::cout << xorstr_("\nEnter license: ") << std::flush;
    std::cin >> key;

    g_sanitizedKey = sanitizeLicense(key);
    if (g_sanitizedKey.empty()) {
        std::cerr << xorstr_("Invalid license format. Exiting... \n ") << std::endl;
        Sleep(3500);
        exit(EXIT_FAILURE);
    }

    
    KeyAuthApp.license(g_sanitizedKey);
    if (!KeyAuthApp.response.success) {
        std::cerr << "\n" << KeyAuthApp.response.message << std::endl;
        Sleep(3500);
        exit(EXIT_FAILURE);
    }

     
    KeyAuthApp.log(g_sanitizedKey + xorstr_(" logged in"));

    
    if (!KeyAuthApp.user_data.subscriptions.empty()) {
        const std::string& expiryStr = KeyAuthApp.user_data.subscriptions[0].expiry;
        time_t expiryTime = stringToTime(expiryStr);
        std::string readableExpiry = timeToReadable(expiryTime);
        std::cout << xorstr_("Subscription expiry: ") << readableExpiry << std::endl;
    }
    else {
        std::cout << xorstr_("No Subscription Found... Odd.. \n ") << std::endl;
    }
     
    g_authMonitorThread = std::jthread([](std::stop_token st) {
        bool disconnectCondition = false;
        bool banCondition = false;
        std::string disconnectMessage;
        std::string banMessage;

        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::seconds(10));

           
            KeyAuthApp.check();
            if (!KeyAuthApp.response.success) {
                disconnectCondition = true;
                disconnectMessage = KeyAuthApp.response.message;
                break;
            }

            KeyAuthApp.license(g_sanitizedKey);
            if (!KeyAuthApp.response.success) {
                disconnectCondition = true;
                disconnectMessage = KeyAuthApp.response.message;
                
                break;
            }

            if (KeyAuthApp.user_data.subscriptions.empty()) {
                disconnectCondition = true;
                disconnectMessage = KeyAuthApp.response.message;
                break;
            }

             
            if (IsDebuggerPresent() || VMProtectIsDebuggerPresent(true)) {
                banCondition = true;
                banMessage = xorstr_("Account has been Disconnected \n.");
                KeyAuthApp.log(xorstr_(" got banned for Debugger"));
                break;
            }

             
            if (!VMProtectIsValidImageCRC()) {
                banCondition = true;
                banMessage = xorstr_("Account has been Disconnected. \n ");
                KeyAuthApp.log(xorstr_(" got banned for modifying exe"));
                break;
            }
        }

        
        if (banCondition) {
            std::cerr << banMessage << std::endl;
            KeyAuthApp.logout();
            Sleep(1500);
            exit(EXIT_FAILURE);
        }
        if (disconnectCondition) {
            std::cerr << disconnectMessage << std::endl;
            KeyAuthApp.logout();
            Sleep(1500);
            exit(EXIT_FAILURE);
        }
        });

    
    if (!VMProtectIsValidImageCRC()) {
        std::cerr << xorstr_("Invalid image CRC. Exiting... \n") << std::endl;
        KeyAuthApp.logout();
        Sleep(1500);
        exit(EXIT_FAILURE);
    }

    std::cout << xorstr_("License validated. Starting cheat... \n ") << std::endl;
}


 







    
  

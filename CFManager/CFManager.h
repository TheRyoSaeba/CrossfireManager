#pragma once
 
 
#include "../Memory/Memory.h"
#include <iostream>
#include <thread>
#include "logging.h"
#include "xorstr.hpp"
#include <Windows.h>
#include <functional>
 
 

#define LOG_INFO(msg)    Logger::Info(xorstr_(msg))
#define LOG_INFO2(msg)    Logger::Info(msg)
#define LOG_SUCCESS(msg) Logger::Success(xorstr_(msg))
#define LOG_WARN(msg)    Logger::Warn(xorstr_(msg))
#define LOG_ERROR(msg)   Logger::Error(msg)
#define LOG_ERROR2(msg)    Logger::Warn(xorstr_(msg))
#define LOG_ERROR3(msg)    Logger::Error(xorstr_(msg))

 const std::string GAME_NAME = xorstr_("Crossfire.exe");
 const std::string SHELLNAME = xorstr_("CShell_x64.dll");

 
 inline auto logFailure = []() {
     std::string info =
         std::string(xorstr_("Failed to Connect to ")) +
         GAME_NAME +
         xorstr_(" Retrying in one second...\n");
     LOG_ERROR(info);  
     };


 
 /**
	 * \Small DMA manager for running some tasks
	 * \param handle
	 */
  class DMAManager {
 public:

	 using CheatTask = std::function<void()>;

	 void AddTaskInternal(const std::string& id, CheatTask task) {
		 if (m_tasks.find(id) != m_tasks.end())
			 return;
		 m_tasks[id] = std::jthread([task](std::stop_token stopToken) {
			 while (!stopToken.stop_requested()) {
				 task();
				 std::this_thread::sleep_for(std::chrono::milliseconds(3));
			 }
			 });
	 }

	 template<typename F, typename... Args>
	 void AddTask(F&& f, Args&&... args) {

		 std::string id = typeid(f).name();

		 auto task = [&]() { f(args...); };
		 AddTaskInternal(id, task);
	 }

	 void StopTask(const std::string& id) {
		 auto it = m_tasks.find(id);
		 if (it != m_tasks.end()) {
			 it->second.request_stop();
			 it->second.join();
			 m_tasks.erase(it);
		 }
	 }

	 void StopAllTasks() {
		 for (auto& [id, thread] : m_tasks) {
			 thread.request_stop();
			 thread.join();
		 }
		 m_tasks.clear();
	 }

 private:
	 std::unordered_map<std::string, std::jthread> m_tasks;
 };

  inline DMAManager DMA;

  

#pragma once
#include "pch.h"
#include "Registry.h"

class c_keys
{
private:
	uint64_t gafAsyncKeyStateExport = 0;
	uint8_t state_bitmap[64] { };
	uint8_t previous_state_bitmap[256 / 8] { };
	uint64_t win32kbase = 0;

	int win_logon_pid = 0;

	c_registry registry;
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

public:
	c_keys() = default;

	~c_keys() = default;

	bool InitKeyboard();

	void UpdateKeys();
	bool IsKeyDown(uint32_t virtual_key_code);
};

struct r_mouse
{
	bool  Init();
	POINT Pos();
	void  LDown();
	void  LUp();

	uintptr_t curPtrVar = 0, qBase = 0, qTail = 0;
	static DWORD pidKernel;
};
extern r_mouse rMouse;


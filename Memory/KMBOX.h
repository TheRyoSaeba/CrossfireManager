#pragma once

#include <Windows.h>
#include <string>
#include <setupapi.h>
#include <devguid.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <math.h>
#include <random>
#include <string>
#include <thread>
#include <stdio.h>
#include "math.h"
 


#pragma comment(lib, "setupapi.lib")

#ifdef KMBOX_EXPORTS
#define KMBXBOX_B_API __declspec(dllexport)
#define KMBXBOX_NET_API __declspec(dllexport)
#else
#define KMBXBOX_B_API __declspec(dllimport)
#define KMBXBOX_NET_API __declspec(dllimport)
#endif

#pragma once

#define 	cmd_connect			0xaf3c2828 // 连接盒子
#define     cmd_mouse_move		0xaede7345 // 鼠标移动
#define		cmd_mouse_left		0x9823AE8D // 鼠标左键控制
#define		cmd_mouse_middle	0x97a3AE8D // 鼠标中键控制
#define		cmd_mouse_right		0x238d8212 // 鼠标右键控制
#define		cmd_mouse_wheel		0xffeead38 // 鼠标滚轮控制
#define     cmd_mouse_automove	0xaede7346 // 鼠标自动模拟人工移动控制
#define     cmd_keyboard_all    0x123c2c2f // 键盘所有参数控制
#define		cmd_reboot			0xaa8855aa // 盒子重启
#define     cmd_bazerMove       0xa238455a // 鼠标贝塞尔移动
#define     cmd_monitor         0x27388020 // 监控盒子上的物理键鼠数据
#define     cmd_debug           0x27382021 // 开启调试信息
#define     cmd_mask_mouse      0x23234343 // 屏蔽物理键鼠
#define     cmd_unmask_all      0x23344343 // 解除屏蔽物理键鼠
#define     cmd_setconfig       0x1d3d3323 // 设置IP配置信息
#define     cmd_setvidpid       0xffed3232 // 设置device端的vidpid
#define     cmd_showpic         0x12334883 // 显示图片

typedef struct
{
	unsigned int  mac;			//盒子的mac地址（必须）
	unsigned int  rand;			//随机值
	unsigned int  indexpts;		//时间戳
	unsigned int  cmd;			//指令码
}cmd_head_t;

typedef struct
{
	unsigned char buff[1024];
}cmd_data_t;
typedef struct
{
	unsigned short buff[512];
}cmd_u16_t;

// 鼠标数据
typedef struct
{
	int button;
	int x;
	int y;
	int wheel;
	int point[10];
}soft_mouse_t;

// 键盘数据
typedef struct
{
	char ctrl;
	char resvel;
	char button[10];
}soft_keyboard_t;

// 客户端数据
typedef struct
{
	cmd_head_t head;
	union {
		cmd_data_t      u8buff;		  //buff
		cmd_u16_t       u16buff;	  //U16
		soft_mouse_t    cmd_mouse;    //鼠标发送指令
		soft_keyboard_t cmd_keyboard; //键盘发送指令
	};
}client_data;

enum
{
	err_creat_socket = -9000,	//创建socket失败
	err_net_version,			//socket版本错误
	err_net_tx,					//socket发送错误
	err_net_rx_timeout,			//socket接收超时
	err_net_cmd,				//命令错误
	err_net_pts,				//时间戳错误
	success = 0,				//正常执行
	usb_dev_tx_timeout,			//USB devic发送失败
};


#pragma pack(1)
typedef struct {
	unsigned char report_id;
	unsigned char buttons;
	short x;
	short y;
	short wheel;
}standard_mouse_report_t;

typedef struct {
	unsigned char report_id;
	unsigned char buttons;
	unsigned char data[10];
}standard_keyboard_report_t;
#pragma pack()

#define KEY_NONE                               0x00
#define KEY_ERRORROLLOVER                      0x01
#define KEY_POSTFAIL                           0x02
#define KEY_ERRORUNDEFINED                     0x03
#define KEY_A                                  0x04
#define KEY_B                                  0x05
#define KEY_C                                  0x06
#define KEY_D                                  0x07 
#define KEY_E                                  0x08
#define KEY_F                                  0x09
#define KEY_G                                  0x0A
#define KEY_H                                  0x0B
#define KEY_I                                  0x0C
#define KEY_J                                  0x0D
#define KEY_K                                  0x0E
#define KEY_L                                  0x0F
#define KEY_M                                  0x10
#define KEY_N                                  0x11
#define KEY_O                                  0x12
#define KEY_P                                  0x13
#define KEY_Q                                  0x14
#define KEY_R                                  0x15
#define KEY_S                                  0x16
#define KEY_T                                  0x17
#define KEY_U                                  0x18
#define KEY_V                                  0x19
#define KEY_W                                  0x1A
#define KEY_X                                  0x1B
#define KEY_Y                                  0x1C
#define KEY_Z                                  0x1D
#define KEY_1_EXCLAMATION_MARK                 0x1E
#define KEY_2_AT                               0x1F
#define KEY_3_NUMBER_SIGN                      0x20
#define KEY_4_DOLLAR                           0x21
#define KEY_5_PERCENT                          0x22
#define KEY_6_CARET                            0x23
#define KEY_7_AMPERSAND                        0x24
#define KEY_8_ASTERISK                         0x25
#define KEY_9_OPARENTHESIS                     0x26
#define KEY_0_CPARENTHESIS                     0x27
#define KEY_ENTER                              0x28
#define KEY_ESCAPE                             0x29
#define KEY_BACKSPACE                          0x2A
#define KEY_TAB                                0x2B
#define KEY_SPACEBAR                           0x2C
#define KEY_MINUS_UNDERSCORE                   0x2D
#define KEY_EQUAL_PLUS                         0x2E
#define KEY_OBRACKET_AND_OBRACE                0x2F
#define KEY_CBRACKET_AND_CBRACE                0x30
#define KEY_BACKSLASH_VERTICAL_BAR             0x31
#define KEY_NONUS_NUMBER_SIGN_TILDE            0x32
#define KEY_SEMICOLON_COLON                    0x33
#define KEY_SINGLE_AND_DOUBLE_QUOTE            0x34
#define KEY_GRAVE ACCENT AND TILDE             0x35
#define KEY_COMMA_AND_LESS                     0x36
#define KEY_DOT_GREATER                        0x37
#define KEY_SLASH_QUESTION                     0x38
#define KEY_CAPS LOCK                          0x39
#define KEY_F1                                 0x3A
#define KEY_F2                                 0x3B
#define KEY_F3                                 0x3C
#define KEY_F4                                 0x3D
#define KEY_F5                                 0x3E
#define KEY_F6                                 0x3F
#define KEY_F7                                 0x40
#define KEY_F8                                 0x41
#define KEY_F9                                 0x42
#define KEY_F10                                0x43
#define KEY_F11                                0x44
#define KEY_F12                                0x45
#define KEY_PRINTSCREEN                        0x46
#define KEY_SCROLL LOCK                        0x47
#define KEY_PAUSE                              0x48
#define KEY_INSERT                             0x49
#define KEY_HOME                               0x4A
#define KEY_PAGEUP                             0x4B
#define KEY_DELETE                             0x4C
#define KEY_END1                               0x4D
#define KEY_PAGEDOWN                           0x4E
#define KEY_RIGHTARROW                         0x4F
#define KEY_LEFTARROW                          0x50
#define KEY_DOWNARROW                          0x51
#define KEY_UPARROW                            0x52
#define KEY_KEYPAD_NUM_LOCK_AND_CLEAR          0x53
#define KEY_KEYPAD_SLASH                       0x54
#define KEY_KEYPAD_ASTERIKS                    0x55
#define KEY_KEYPAD_MINUS                       0x56
#define KEY_KEYPAD_PLUS                        0x57
#define KEY_KEYPAD_ENTER                       0x58
#define KEY_KEYPAD_1_END                       0x59
#define KEY_KEYPAD_2_DOWN_ARROW                0x5A
#define KEY_KEYPAD_3_PAGEDN                    0x5B
#define KEY_KEYPAD_4_LEFT_ARROW                0x5C
#define KEY_KEYPAD_5                           0x5D
#define KEY_KEYPAD_6_RIGHT_ARROW               0x5E
#define KEY_KEYPAD_7_HOME                      0x5F
#define KEY_KEYPAD_8_UP_ARROW                  0x60
#define KEY_KEYPAD_9_PAGEUP                    0x61
#define KEY_KEYPAD_0_INSERT                    0x62
#define KEY_KEYPAD_DECIMAL_SEPARATOR_DELETE    0x63
#define KEY_NONUS_BACK_SLASH_VERTICAL_BAR      0x64
#define KEY_APPLICATION                        0x65
#define KEY_POWER                              0x66
#define KEY_KEYPAD_EQUAL                       0x67
#define KEY_F13                                0x68
#define KEY_F14                                0x69
#define KEY_F15                                0x6A
#define KEY_F16                                0x6B
#define KEY_F17                                0x6C
#define KEY_F18                                0x6D
#define KEY_F19                                0x6E
#define KEY_F20                                0x6F
#define KEY_F21                                0x70
#define KEY_F22                                0x71
#define KEY_F23                                0x72
#define KEY_F24                                0x73
#define KEY_EXECUTE                            0x74
#define KEY_HELP                               0x75
#define KEY_MENU                               0x76
#define KEY_SELECT                             0x77
#define KEY_STOP                               0x78
#define KEY_AGAIN                              0x79
#define KEY_UNDO                               0x7A
#define KEY_CUT                                0x7B
#define KEY_COPY                               0x7C
#define KEY_PASTE                              0x7D
#define KEY_FIND                               0x7E
#define KEY_MUTE                               0x7F
#define KEY_VOLUME_UP                          0x80
#define KEY_VOLUME_DOWN                        0x81
#define KEY_LOCKING_CAPS_LOCK                  0x82
#define KEY_LOCKING_NUM_LOCK                   0x83
#define KEY_LOCKING_SCROLL_LOCK                0x84
#define KEY_KEYPAD_COMMA                       0x85
#define KEY_KEYPAD_EQUAL_SIGN                  0x86
#define KEY_INTERNATIONAL1                     0x87
#define KEY_INTERNATIONAL2                     0x88
#define KEY_INTERNATIONAL3                     0x89
#define KEY_INTERNATIONAL4                     0x8A
#define KEY_INTERNATIONAL5                     0x8B
#define KEY_INTERNATIONAL6                     0x8C
#define KEY_INTERNATIONAL7                     0x8D
#define KEY_INTERNATIONAL8                     0x8E
#define KEY_INTERNATIONAL9                     0x8F
#define KEY_LANG1                              0x90
#define KEY_LANG2                              0x91
#define KEY_LANG3                              0x92
#define KEY_LANG4                              0x93
#define KEY_LANG5                              0x94
#define KEY_LANG6                              0x95
#define KEY_LANG7                              0x96
#define KEY_LANG8                              0x97
#define KEY_LANG9                              0x98
#define KEY_ALTERNATE_ERASE                    0x99
#define KEY_SYSREQ                             0x9A
#define KEY_CANCEL                             0x9B
#define KEY_CLEAR                              0x9C
#define KEY_PRIOR                              0x9D
#define KEY_RETURN                             0x9E
#define KEY_SEPARATOR                          0x9F
#define KEY_OUT                                0xA0
#define KEY_OPER                               0xA1
#define KEY_CLEAR_AGAIN                        0xA2
#define KEY_CRSEL                              0xA3
#define KEY_EXSEL                              0xA4
#define KEY_KEYPAD_00                          0xB0
#define KEY_KEYPAD_000                         0xB1
#define KEY_THOUSANDS_SEPARATOR                0xB2
#define KEY_DECIMAL_SEPARATOR                  0xB3
#define KEY_CURRENCY_UNIT                      0xB4
#define KEY_CURRENCY_SUB_UNIT                  0xB5
#define KEY_KEYPAD_OPARENTHESIS                0xB6
#define KEY_KEYPAD_CPARENTHESIS                0xB7
#define KEY_KEYPAD_OBRACE                      0xB8
#define KEY_KEYPAD_CBRACE                      0xB9
#define KEY_KEYPAD_TAB                         0xBA
#define KEY_KEYPAD_BACKSPACE                   0xBB
#define KEY_KEYPAD_A                           0xBC
#define KEY_KEYPAD_B                           0xBD
#define KEY_KEYPAD_C                           0xBE
#define KEY_KEYPAD_D                           0xBF
#define KEY_KEYPAD_E                           0xC0
#define KEY_KEYPAD_F                           0xC1
#define KEY_KEYPAD_XOR                         0xC2
#define KEY_KEYPAD_CARET                       0xC3
#define KEY_KEYPAD_PERCENT                     0xC4
#define KEY_KEYPAD_LESS                        0xC5
#define KEY_KEYPAD_GREATER                     0xC6
#define KEY_KEYPAD_AMPERSAND                   0xC7
#define KEY_KEYPAD_LOGICAL_AND                 0xC8
#define KEY_KEYPAD_VERTICAL_BAR                0xC9
#define KEY_KEYPAD_LOGIACL_OR                  0xCA
#define KEY_KEYPAD_COLON                       0xCB
#define KEY_KEYPAD_NUMBER_SIGN                 0xCC
#define KEY_KEYPAD_SPACE                       0xCD
#define KEY_KEYPAD_AT                          0xCE
#define KEY_KEYPAD_EXCLAMATION_MARK            0xCF
#define KEY_KEYPAD_MEMORY_STORE                0xD0
#define KEY_KEYPAD_MEMORY_RECALL               0xD1
#define KEY_KEYPAD_MEMORY_CLEAR                0xD2
#define KEY_KEYPAD_MEMORY_ADD                  0xD3
#define KEY_KEYPAD_MEMORY_SUBTRACT             0xD4
#define KEY_KEYPAD_MEMORY_MULTIPLY             0xD5
#define KEY_KEYPAD_MEMORY_DIVIDE               0xD6
#define KEY_KEYPAD_PLUSMINUS                   0xD7
#define KEY_KEYPAD_CLEAR                       0xD8
#define KEY_KEYPAD_CLEAR_ENTRY                 0xD9
#define KEY_KEYPAD_BINARY                      0xDA
#define KEY_KEYPAD_OCTAL                       0xDB
#define KEY_KEYPAD_DECIMAL                     0xDC
#define KEY_KEYPAD_HEXADECIMAL                 0xDD
#define KEY_LEFTCONTROL                        0xE0
#define KEY_LEFTSHIFT                          0xE1
#define KEY_LEFTALT                            0xE2
#define KEY_LEFT_GUI                           0xE3
#define KEY_RIGHTCONTROL                       0xE4
#define KEY_RIGHTSHIFT                         0xE5
#define KEY_RIGHTALT                           0xE6
#define KEY_RIGHT_GUI                          0xE7


#define BIT0 0X01
#define BIT1 0X02
#define BIT2 0X04
#define BIT3 0X08
#define BIT4 0X10
#define BIT5 0X20
#define BIT6 0X40
#define BIT7 0X80
#pragma once

 


// Function declarations
KMBXBOX_B_API std::string find_port(const std::string& targetDescription);
KMBXBOX_B_API bool open_port(HANDLE& hSerial, const char* portName, DWORD baudRate);
KMBXBOX_B_API void send_command(HANDLE hSerial, const std::string& command);

 
class KMBXBOX_B_API KmBoxBManager {
private:
	HANDLE hSerial;
	 
public:
	 
	int init();
	void km_move(int X, int Y);
	int km_getpos(int& x, int& y);
	void km_move_auto(int X, int Y, int runtime);
	void km_click();
	bool km_right(bool down);
	void lock_mx();
	void lock_my();
	void lock_mr();
	void unlock_mx();
	void unlock_mr();
	void unlock_my();
 
	 
	HANDLE getSerialHandle() const;
	 
};

class KMBXBOX_NET_API KmBoxMouse {
public:
	soft_mouse_t MouseData{};


	int Move(int x, int y);


	int Move_Auto(int x, int y, int runtime);


	int Left(bool down);


	int Right(bool down);


	int Middle(bool down);
};
class KMBXBOX_NET_API KmBoxKeyBoard
{
public:
	std::thread t_Listen;
	WORD MonitorPort;
	SOCKET s_ListenSocket = 0;
	bool ListenerRuned = false;
public:
	standard_keyboard_report_t hw_Keyboard;
	standard_mouse_report_t hw_Mouse;
public:
	~KmBoxKeyBoard();
	void ListenThread();
	int StartMonitor(WORD Port);
	void EndMonitor();
public:
	bool GetKeyState(WORD vKey);
};

class KMBXBOX_NET_API KmBoxNetManager
{
private:
	SOCKADDR_IN AddrServer;
	SOCKET s_Client = 0;
	client_data ReceiveData;
	client_data PostData;
private:
	int NetHandler();
	int SendData(int DataLength);
public:
	~KmBoxNetManager();
	// 初始化设备
	int InitDevice(const std::string& IP, WORD Port, const std::string& Mac);
	// 重启设备
	int RebootDevice();
	// 设置设备s
	int SetConfig(const std::string& IP, WORD Port);
public:
	friend class KmBoxMouse;
	KmBoxMouse Mouse;
	friend class KmBoxKeyBoard;
	KmBoxKeyBoard KeyBoard;
};


 

// Global instance of KmBoxBManager
extern KMBXBOX_B_API KmBoxBManager kmBoxBMgr;
extern KMBXBOX_NET_API KmBoxNetManager KmBoxNETMgr;
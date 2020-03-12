#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include "services/tc.hpp"
#include "services/fan.hpp"
#include "libstratosphere/dmntcht.hpp"

//Common
Thread t0;
Thread t1;
Thread t2;
Thread t3;
Thread t4;
Thread t5;
Thread t6;
u64 systemtickfrequency = 19200000;
bool threadexit = false;
bool threadexit2 = false;
u64 refreshrate = 1;
FanController g_ICon;

//Mini mode
char Variables[256];

//Checks
Result smCheck = 1;
Result clkrstCheck = 1;
Result nvCheck = 1;
Result pcvCheck = 1;
Result tsCheck = 1;
Result fanCheck = 1;
Result tcCheck = 1;
Result Hinted = 1;
Result pmdmntCheck = 1;
Result dmntchtCheck = 1;

//Temperatures
s32 SoC_temperaturemiliC = 0;
float SoC_temperatureC = 0;
s32 PCB_temperaturemiliC = 0;
float PCB_temperatureC = 0;
s32 skin_temperaturemiliC = 0;
float skin_temperatureC = 0;
char SoCPCB_temperature_c[64];
char skin_temperature_c[32];

//CPU Usage
double percent = 0;
u64 idletick_a0 = 0;
u64 idletick_a1 = 0;
u64 idletick_a2 = 0;
u64 idletick_a3 = 0;
u64 idletick_b0 = 0;
u64 idletick_b1 = 0;
u64 idletick_b2 = 0;
u64 idletick_b3 = 0;
u64 idletick0 = 19200000;
u64 idletick1 = 19200000;
u64 idletick2 = 19200000;
u64 idletick3 = 19200000;
char CPU_Usage0[32];
char CPU_Usage1[32];
char CPU_Usage2[32];
char CPU_Usage3[32];
char CPU_compressed_c[128];

//Frequency
///CPU
u32 CPU_Hz = 0;
float CPU_Hz_f = 0;
char CPU_Hz_c[32];
///GPU
u32 GPU_Hz = 0;
float GPU_Hz_f = 0;
char GPU_Hz_c[32];
///RAM
u32 RAM_Hz = 0;
float RAM_Hz_f = 0;
char RAM_Hz_c[32];

//RAM Size
char RAM_all_c[64];
char RAM_application_c[64];
char RAM_applet_c[64];
char RAM_system_c[64];
char RAM_systemunsafe_c[64];
char RAM_compressed_c[320];
char RAM_var_compressed_c[320];
u64 RAM_Total_all_u = 0;
float RAM_Total_all_f = 0;
u64 RAM_Total_application_u = 0;
float RAM_Total_application_f = 0;
u64 RAM_Total_applet_u = 0;
float RAM_Total_applet_f = 0;
u64 RAM_Total_system_u = 0;
float RAM_Total_system_f = 0;
u64 RAM_Total_systemunsafe_u = 0;
float RAM_Total_systemunsafe_f = 0;
u64 RAM_Used_all_u = 0;
float RAM_Used_all_f = 0;
u64 RAM_Used_application_u = 0;
float RAM_Used_application_f = 0;
u64 RAM_Used_applet_u = 0;
float RAM_Used_applet_f = 0;
u64 RAM_Used_system_u = 0;
float RAM_Used_system_f = 0;
u64 RAM_Used_systemunsafe_u = 0;
float RAM_Used_systemunsafe_f = 0;

//Fan
float Rotation_SpeedLevel_f = 0;
float Rotation_SpeedLevel_percent = 0;
char Rotation_SpeedLevel_c[64];

//GPU Usage
u32 fd = 0;
u32 GPU_Load_u = 0;
float GPU_Load_percent = 0;
char GPU_Load_c[32];
float GPU_Load_max = 1000;

//NX-FPS
bool GameRunning = false;
uint8_t check = 0;
bool SaltySD = false;
uintptr_t FPSaddress = 0x0;
uintptr_t FPSavgaddress = 0x0;
char FPS_c[32];
uint8_t FPS = 0xFE;
char FPSavg_c[32];
float FPSavg = 255;
char FPS_compressed_c[64];
char FPS_var_compressed_c[64];

//Check if SaltyNX is working
bool CheckPort () {
	Result ret;
	Handle saltysd;
    for (int i = 0; i < 200; i++)
    {
        ret = svcConnectToNamedPort(&saltysd, "InjectServ");
        svcSleepThread(1000*1000);
        
        if (!ret) break;
    }
	svcCloseHandle(saltysd);
	if (ret != 0x0) return false;
	else return true;
}

void CheckIfGameRunning() {
	while (threadexit2 == false) {
		if (R_SUCCEEDED(dmntchtCheck)) {
			Result rc = 1;
			uint64_t PID = 0;
			rc = pmdmntGetApplicationProcessId(&PID);
			if (R_FAILED(rc)) {
				if (check == 0) {
					remove("sdmc:/SaltySD/FPSoffset.hex");
				}
				check = 1;
				GameRunning = false;
			}
			else if (GameRunning == false) {
				svcSleepThread(1000*1000*1000);
				FILE* FPSoffset = fopen("sdmc:/SaltySD/FPSoffset.hex", "rb");
				if ((FPSoffset != NULL)) {
					dmntchtForceOpenCheatProcess();
					fread(&FPSaddress, 0x5, 1, FPSoffset);
					FPSavgaddress = FPSaddress - 0x8;
					fclose(FPSoffset);
					GameRunning = true;
					check = 0;
				}
			}
		}
		svcSleepThread(1000*1000*1000);
	}
}

//Check for input outside of FPS limitations
void CheckButtons() {
	while (threadexit == false) {
		hidScanInput();
		u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
		if (kHeld & KEY_ZR) {
			hidScanInput();
			u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
			if (kHeld & KEY_R) {
				hidScanInput();
				u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
				if (kHeld & KEY_DDOWN) {
					TeslaFPS = 1;
					refreshrate = 1;
					systemtickfrequency = 19200000;
				}
				else if (kHeld & KEY_DUP) {
					TeslaFPS = 5;
					refreshrate = 5;
					systemtickfrequency = 3840000;
				}
			}
		}
		svcSleepThread(100*1000*1000);
	}
}

//Stuff that doesn't need multithreading
void Misc() {
	while (threadexit == false) {
		
		// CPU, GPU and RAM Frequency
		if (R_SUCCEEDED(clkrstCheck)) {
			ClkrstSession cpuSession;
			clkrstOpenSession(&cpuSession, PcvModuleId_CpuBus, 3);
			clkrstGetClockRate(&cpuSession, &CPU_Hz);
			clkrstCloseSession(&cpuSession);
			clkrstOpenSession(&cpuSession, PcvModuleId_GPU, 3);
			clkrstGetClockRate(&cpuSession, &GPU_Hz);
			clkrstCloseSession(&cpuSession);
			clkrstOpenSession(&cpuSession, PcvModuleId_EMC, 3);
			clkrstGetClockRate(&cpuSession, &RAM_Hz);
			clkrstCloseSession(&cpuSession);
		}
		else if (R_SUCCEEDED(pcvCheck)) {
			pcvGetClockRate(PcvModule_CpuBus, &CPU_Hz);
			pcvGetClockRate(PcvModule_GPU, &GPU_Hz);
			pcvGetClockRate(PcvModule_EMC, &RAM_Hz);
		}
		
		//Temperatures
		if (R_SUCCEEDED(tsCheck)) {
			tsGetTemperatureMilliC(TsLocation_External, &SoC_temperaturemiliC);
			tsGetTemperatureMilliC(TsLocation_Internal, &PCB_temperaturemiliC);
		}
		if (R_SUCCEEDED(tcCheck)) tcGetSkinTemperatureMilliC(&skin_temperaturemiliC);
		
		//RAM Memory Used
		if (R_SUCCEEDED(Hinted)) {
			svcGetSystemInfo(&RAM_Total_application_u, 0, INVALID_HANDLE, 0);
			svcGetSystemInfo(&RAM_Total_applet_u, 0, INVALID_HANDLE, 1);
			svcGetSystemInfo(&RAM_Total_system_u, 0, INVALID_HANDLE, 2);
			svcGetSystemInfo(&RAM_Total_systemunsafe_u, 0, INVALID_HANDLE, 3);
			svcGetSystemInfo(&RAM_Used_application_u, 1, INVALID_HANDLE, 0);
			svcGetSystemInfo(&RAM_Used_applet_u, 1, INVALID_HANDLE, 1);
			svcGetSystemInfo(&RAM_Used_system_u, 1, INVALID_HANDLE, 2);
			svcGetSystemInfo(&RAM_Used_systemunsafe_u, 1, INVALID_HANDLE, 3);
		}
		
		//Fan
		if (R_SUCCEEDED(fanCheck)) fanControllerGetRotationSpeedLevel(&g_ICon, &Rotation_SpeedLevel_f);
		
		//GPU Load
		if (R_SUCCEEDED(nvCheck)) nvIoctl(fd, 0x80044715, &GPU_Load_u);
		
		//FPS
		if (GameRunning == true) {
			dmntchtReadCheatProcessMemory(FPSaddress, &FPS, 0x1);
			dmntchtReadCheatProcessMemory(FPSavgaddress, &FPSavg, 0x4);
		}
		
		// 1 sec interval
		svcSleepThread(1000*1000*1000 / refreshrate);
	}
}

//Check each core for idled ticks in 1s intervals, they cannot read info about other core than they are assigned
void CheckCore0() {
	while (threadexit == false) {
		svcGetInfo(&idletick_b0, InfoType_IdleTickCount, INVALID_HANDLE, 0);
		svcSleepThread(1000*1000*1000 / refreshrate);
		svcGetInfo(&idletick_a0, InfoType_IdleTickCount, INVALID_HANDLE, 0);
		idletick0 = idletick_a0 - idletick_b0;
	}
}

void CheckCore1() {
	while (threadexit == false) {
		svcGetInfo(&idletick_b1, InfoType_IdleTickCount, INVALID_HANDLE, 1);
		svcSleepThread(1000*1000*1000 / refreshrate);
		svcGetInfo(&idletick_a1, InfoType_IdleTickCount, INVALID_HANDLE, 1);
		idletick1 = idletick_a1 - idletick_b1;
	}
}

void CheckCore2() {
	while (threadexit == false) {
		svcGetInfo(&idletick_b2, InfoType_IdleTickCount, INVALID_HANDLE, 2);
		svcSleepThread(1000*1000*1000 / refreshrate);
		svcGetInfo(&idletick_a2, InfoType_IdleTickCount, INVALID_HANDLE, 2);
		idletick2 = idletick_a2 - idletick_b2;
	}
}

void CheckCore3() {
	while (threadexit == false) {
		svcGetInfo(&idletick_b3, InfoType_IdleTickCount, INVALID_HANDLE, 3);
		svcSleepThread(1000*1000*1000 / refreshrate);
		svcGetInfo(&idletick_a3, InfoType_IdleTickCount, INVALID_HANDLE, 3);
		idletick3 = idletick_a3 - idletick_b3;
	}
}

//Start reading all stats
void StartThreads() {
	threadCreate(&t0, CheckCore0, NULL, NULL, 0x100, 0x3B, 0);
	threadCreate(&t1, CheckCore1, NULL, NULL, 0x100, 0x3B, 1);
	threadCreate(&t2, CheckCore2, NULL, NULL, 0x100, 0x3B, 2);
	threadCreate(&t3, CheckCore3, NULL, NULL, 0x100, 0x3F, 3);
	threadCreate(&t4, Misc, NULL, NULL, 0x100, 0x3A, -2);
	threadCreate(&t5, CheckButtons, NULL, NULL, 0x200, 0x39, -2);
	threadStart(&t0);
	threadStart(&t1);
	threadStart(&t2);
	threadStart(&t3);
	threadStart(&t4);
	threadStart(&t5);
}

//End reading all stats
void CloseThreads() {
	threadexit = true;
	threadWaitForExit(&t0);
	threadWaitForExit(&t1);
	threadWaitForExit(&t2);
	threadWaitForExit(&t3);
	threadWaitForExit(&t4);
	threadWaitForExit(&t5);
	threadClose(&t0);
	threadClose(&t1);
	threadClose(&t2);
	threadClose(&t3);
	threadClose(&t4);
	threadClose(&t5);
	threadexit = false;
}

//Separate functions dedicated to "FPS Counter" mode
void FPSCounter() {
	while (threadexit == false) {
		dmntchtReadCheatProcessMemory(FPSavgaddress, &FPSavg, 0x4);
		
		// 1 sec interval
		svcSleepThread(1000*1000*1000 / refreshrate);
	}
}

void StartFPSCounterThread() {
	threadCreate(&t0, FPSCounter, NULL, NULL, 0x100, 0x3F, 3);
	threadStart(&t0);
}

void EndFPSCounterThread() {
	threadexit = true;
	threadWaitForExit(&t0);
	threadClose(&t0);
	threadexit = false;
}

//FPS Counter mode
class com_FPS : public tsl::Gui {
public:
    com_FPS() { }

    virtual tsl::elm::Element* createUI() override {
		auto rootFrame = new tsl::elm::OverlayFrame("", "");

		auto Status = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
				static uint8_t avg = 0;
				if (FPSavg < 10) avg = 0;
				if (FPSavg >= 10) avg = 23;
				if (FPSavg >= 100) avg = 46;
				renderer->drawRect(0, 0, tsl::cfg::FramebufferWidth - 370 + avg, 50, a(0x7111));
				renderer->drawString(FPSavg_c, false, 5, 40, 40, renderer->a(0xFFFF));
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {
		///FPS
		snprintf(FPSavg_c, sizeof FPSavg_c, "%2.1f", FPSavg);
		
	}
	virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		if (keysHeld & KEY_LSTICK) {
			if (keysHeld & KEY_RSTICK) {
				EndFPSCounterThread();
				tsl::goBack();
				return true;
			}
		}
		return false;
	}
};

//Full mode
class FullOverlay : public tsl::Gui {
public:
    FullOverlay() { }

    virtual tsl::elm::Element* createUI() override {
		auto rootFrame = new tsl::elm::OverlayFrame("状态监视器", "v0.5");

		auto Status = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			
			//Print strings
			///CPU
			renderer->drawString("CPU使用率:", false, 20, 120, 20, renderer->a(0xFFFF));
			if (R_SUCCEEDED(clkrstCheck) || R_SUCCEEDED(pcvCheck)) renderer->drawString(CPU_Hz_c, false, 20, 155, 15, renderer->a(0xFFFF));
			renderer->drawString(CPU_compressed_c, false, 20, 185, 15, renderer->a(0xFFFF));
			
			///GPU
			if (R_SUCCEEDED(clkrstCheck) || R_SUCCEEDED(pcvCheck) || R_SUCCEEDED(nvCheck)) {
				
				renderer->drawString("GPU使用率:", false, 20, 285, 20, renderer->a(0xFFFF));
				if (R_SUCCEEDED(clkrstCheck) || R_SUCCEEDED(pcvCheck)) renderer->drawString(GPU_Hz_c, false, 20, 320, 15, renderer->a(0xFFFF));
				if (R_SUCCEEDED(nvCheck)) renderer->drawString(GPU_Load_c, false, 20, 335, 15, renderer->a(0xFFFF));
				
			}
			
			///RAM
			if (R_SUCCEEDED(clkrstCheck) || R_SUCCEEDED(pcvCheck) || R_SUCCEEDED(Hinted)) {
				
				renderer->drawString("RAM使用率:", false, 20, 375, 20, renderer->a(0xFFFF));
				if (R_SUCCEEDED(clkrstCheck) || R_SUCCEEDED(pcvCheck)) renderer->drawString(RAM_Hz_c, false, 20, 410, 15, renderer->a(0xFFFF));
				if (R_SUCCEEDED(Hinted)) {
					renderer->drawString(RAM_compressed_c, false, 20, 440, 15, renderer->a(0xFFFF));
					renderer->drawString(RAM_var_compressed_c, false, 140, 440, 15, renderer->a(0xFFFF));
				}
			}
			
			///Thermal
			if (R_SUCCEEDED(tsCheck) || R_SUCCEEDED(tcCheck) || R_SUCCEEDED(fanCheck)) {
				renderer->drawString("温度:", false, 20, 540, 20, renderer->a(0xFFFF));
				if (R_SUCCEEDED(tsCheck)) renderer->drawString(SoCPCB_temperature_c, false, 20, 575, 15, renderer->a(0xFFFF));
				if (R_SUCCEEDED(tcCheck)) renderer->drawString(skin_temperature_c, false, 20, 605, 15, renderer->a(0xFFFF));
				if (R_SUCCEEDED(fanCheck)) renderer->drawString(Rotation_SpeedLevel_c, false, 20, 620, 15, renderer->a(0xFFFF));
			}
			
			///FPS
			if (GameRunning == true) {
				renderer->drawString(FPS_compressed_c, false, 235, 120, 20, renderer->a(0xFFFF));
				renderer->drawString(FPS_var_compressed_c, false, 295, 120, 20, renderer->a(0xFFFF));
			}
			
			if (refreshrate == 5) renderer->drawString("按住左摇杆和右摇杆退出\n按住ZR + R + 方向键下 可减慢刷新速度", false, 20, 675, 15, renderer->a(0xFFFF));
			if (refreshrate == 1) renderer->drawString("按住左摇杆和右摇杆退出\n按住ZR + R + 方向键上 可加快刷新速度", false, 20, 675, 15, renderer->a(0xFFFF));
		
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {
		if (TeslaFPS == 60) TeslaFPS = 1;
		//In case of getting more than systemtickfrequency in idle, make it equal to systemtickfrequency to get 0% as output and nothing less
		//This is because making each loop also takes time, which is not considered because this will take also additional time
		if (idletick0 > systemtickfrequency) idletick0 = systemtickfrequency;
		if (idletick1 > systemtickfrequency) idletick1 = systemtickfrequency;
		if (idletick2 > systemtickfrequency) idletick2 = systemtickfrequency;
		if (idletick3 > systemtickfrequency) idletick3 = systemtickfrequency;
		
		//Make stuff ready to print
		///CPU
		CPU_Hz_f = (float)CPU_Hz / 1000000;
		snprintf(CPU_Hz_c, sizeof CPU_Hz_c, "频率: %.1f MHz", CPU_Hz_f);
		percent = (double) (((double)systemtickfrequency - (double)idletick0) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage0, sizeof CPU_Usage0, "核心 #0: %.2f%s", percent, "%");
		percent = (double) (((double)systemtickfrequency - (double)idletick1) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage1, sizeof CPU_Usage1, "核心 #1: %.2f%s", percent, "%");
		percent = (double) (((double)systemtickfrequency - (double)idletick2) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage2, sizeof CPU_Usage2, "核心 #2: %.2f%s", percent, "%");
		percent = (double) (((double)systemtickfrequency - (double)idletick3) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage3, sizeof CPU_Usage3, "核心 #3: %.2f%s", percent, "%");
		snprintf(CPU_compressed_c, sizeof CPU_compressed_c, "%s\n%s\n%s\n%s", CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3);
		
		///GPU
		GPU_Hz_f = (float)GPU_Hz / 1000000;
		snprintf(GPU_Hz_c, sizeof GPU_Hz_c, "频率: %.1f MHz", GPU_Hz_f);
		GPU_Load_percent = (float)GPU_Load_u / GPU_Load_max * 100;
		snprintf(GPU_Load_c, sizeof GPU_Load_c, "Load: %.1f%s", GPU_Load_percent, "%");
		
		///RAM
		RAM_Hz_f = (float)RAM_Hz / 1000000;
		snprintf(RAM_Hz_c, sizeof RAM_Hz_c, "频率: %.1f MHz", RAM_Hz_f);
		RAM_Total_application_f = (float)RAM_Total_application_u / 1024 / 1024;
		RAM_Total_applet_f = (float)RAM_Total_applet_u / 1024 / 1024;
		RAM_Total_system_f = (float)RAM_Total_system_u / 1024 / 1024;
		RAM_Total_systemunsafe_f = (float)RAM_Total_systemunsafe_u / 1024 / 1024;
		RAM_Total_all_f = RAM_Total_application_f + RAM_Total_applet_f + RAM_Total_system_f + RAM_Total_systemunsafe_f;
		RAM_Used_application_f = (float)RAM_Used_application_u / 1024 / 1024;
		RAM_Used_applet_f = (float)RAM_Used_applet_u / 1024 / 1024;
		RAM_Used_system_f = (float)RAM_Used_system_u / 1024 / 1024;
		RAM_Used_systemunsafe_f = (float)RAM_Used_systemunsafe_u / 1024 / 1024;
		RAM_Used_all_f = RAM_Used_application_f + RAM_Used_applet_f + RAM_Used_system_f + RAM_Used_systemunsafe_f;
		snprintf(RAM_all_c, sizeof RAM_all_c, "总计:");
		snprintf(RAM_application_c, sizeof RAM_application_c, "应用程序:");
		snprintf(RAM_applet_c, sizeof RAM_applet_c, "小程序:");
		snprintf(RAM_system_c, sizeof RAM_system_c, "系统:");
		snprintf(RAM_systemunsafe_c, sizeof RAM_systemunsafe_c, "系统保留:");
		snprintf(RAM_compressed_c, sizeof RAM_compressed_c, "%s\n%s\n%s\n%s\n%s", RAM_all_c, RAM_application_c, RAM_applet_c, RAM_system_c, RAM_systemunsafe_c);
		snprintf(RAM_all_c, sizeof RAM_all_c, "%4.2f / %4.2f MB", RAM_Used_all_f, RAM_Total_all_f);
		snprintf(RAM_application_c, sizeof RAM_application_c, "%4.2f / %4.2f MB", RAM_Used_application_f, RAM_Total_application_f);
		snprintf(RAM_applet_c, sizeof RAM_applet_c, "%4.2f / %4.2f MB", RAM_Used_applet_f, RAM_Total_applet_f);
		snprintf(RAM_system_c, sizeof RAM_system_c, "%4.2f / %4.2f MB", RAM_Used_system_f, RAM_Total_system_f);
		snprintf(RAM_systemunsafe_c, sizeof RAM_systemunsafe_c, "%4.2f / %4.2f MB", RAM_Used_systemunsafe_f, RAM_Total_systemunsafe_f);
		snprintf(RAM_var_compressed_c, sizeof RAM_var_compressed_c, "%s\n%s\n%s\n%s\n%s", RAM_all_c, RAM_application_c, RAM_applet_c, RAM_system_c, RAM_systemunsafe_c);
		
		///Thermal
		SoC_temperatureC = (float)SoC_temperaturemiliC / 1000;
		PCB_temperatureC = (float)PCB_temperaturemiliC / 1000;
		snprintf(SoCPCB_temperature_c, sizeof SoCPCB_temperature_c, "芯片: %2.2f \u00B0C\nPCB: %2.2f \u00B0C", SoC_temperatureC, PCB_temperatureC);
		skin_temperatureC = (float)skin_temperaturemiliC / 1000;
		snprintf(skin_temperature_c, sizeof skin_temperature_c, "外表: %2.2f \u00B0C", skin_temperatureC);
		Rotation_SpeedLevel_percent = Rotation_SpeedLevel_f * 100;
		snprintf(Rotation_SpeedLevel_c, sizeof Rotation_SpeedLevel_c, "风扇: %2.2f%s", Rotation_SpeedLevel_percent, "%");
		
		///FPS
		snprintf(FPS_c, sizeof FPS_c, "PFPS:"); //Pushed Frames Per Second
		snprintf(FPSavg_c, sizeof FPSavg_c, "FPS:"); //Frames Per Second calculated from averaged frametime 
		snprintf(FPS_compressed_c, sizeof FPS_compressed_c, "%s\n%s", FPS_c, FPSavg_c);
		snprintf(FPS_c, sizeof FPS_c, "%u", FPS);
		snprintf(FPSavg_c, sizeof FPSavg_c, "%2.2f", FPSavg);
		snprintf(FPS_var_compressed_c, sizeof FPS_compressed_c, "%s\n%s", FPS_c, FPSavg_c);
		
	}
    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (keysHeld & KEY_LSTICK) {
			if (keysHeld & KEY_RSTICK) {
				CloseThreads();
				tsl::goBack();
				return true;
			}
		}
		return false;
    }
};

//Mini mode
class MiniOverlay : public tsl::Gui {
public:
    MiniOverlay() { }

    virtual tsl::elm::Element* createUI() override {
		auto rootFrame = new tsl::elm::OverlayFrame("", "");
		

		auto Status = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			
			if (GameRunning == false) renderer->drawRect(0, 0, tsl::cfg::FramebufferWidth - 150, 80, a(0x7111));
			else renderer->drawRect(0, 0, tsl::cfg::FramebufferWidth - 150, 110, a(0x7111));
			
			//Print strings
			///CPU
			if (GameRunning == true) renderer->drawString("CPU\nGPU\n内存\nTEMP\n风扇\nPFPS\nFPS", false, 0, 15, 15, renderer->a(0xFFFF));
			else renderer->drawString("CPU\nGPU\n内存\nTEMP\n风扇", false, 0, 15, 15, renderer->a(0xFFFF));
			
			///GPU
			renderer->drawString(Variables, false, 60, 15, 15, renderer->a(0xFFFF));
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {
		if (TeslaFPS == 60) TeslaFPS = 1;
		//In case of getting more than systemtickfrequency in idle, make it equal to systemtickfrequency to get 0% as output and nothing less
		//This is because making each loop also takes time, which is not considered because this will take also additional time
		if (idletick0 > systemtickfrequency) idletick0 = systemtickfrequency;
		if (idletick1 > systemtickfrequency) idletick1 = systemtickfrequency;
		if (idletick2 > systemtickfrequency) idletick2 = systemtickfrequency;
		if (idletick3 > systemtickfrequency) idletick3 = systemtickfrequency;
		
		//Make stuff ready to print
		///CPU
		CPU_Hz_f = (float)CPU_Hz / 1000000;
		percent = (double) (((double)systemtickfrequency - (double)idletick0) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage0, sizeof CPU_Usage0, "%.0f%s", percent, "%");
		percent = (double) (((double)systemtickfrequency - (double)idletick1) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage1, sizeof CPU_Usage1, "%.0f%s", percent, "%");
		percent = (double) (((double)systemtickfrequency - (double)idletick2) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage2, sizeof CPU_Usage2, "%.0f%s", percent, "%");
		percent = (double) (((double)systemtickfrequency - (double)idletick3) / ((double)systemtickfrequency)) * 100;
		snprintf(CPU_Usage3, sizeof CPU_Usage3, "%.0f%s", percent, "%");
		snprintf(CPU_compressed_c, sizeof CPU_compressed_c, "[%s,%s,%s,%s]@%.1f", CPU_Usage0, CPU_Usage1, CPU_Usage2, CPU_Usage3, CPU_Hz_f);
		
		///GPU
		GPU_Hz_f = (float)GPU_Hz / 1000000;
		GPU_Load_percent = (float)GPU_Load_u / GPU_Load_max * 100;
		snprintf(GPU_Load_c, sizeof GPU_Load_c, "%.1f%s@%.1f", GPU_Load_percent, "%", GPU_Hz_f);
		
		///RAM
		RAM_Hz_f = (float)RAM_Hz / 1000000;
		RAM_Total_application_f = (float)RAM_Total_application_u / 1024 / 1024;
		RAM_Total_applet_f = (float)RAM_Total_applet_u / 1024 / 1024;
		RAM_Total_system_f = (float)RAM_Total_system_u / 1024 / 1024;
		RAM_Total_systemunsafe_f = (float)RAM_Total_systemunsafe_u / 1024 / 1024;
		RAM_Total_all_f = RAM_Total_application_f + RAM_Total_applet_f + RAM_Total_system_f + RAM_Total_systemunsafe_f;
		RAM_Used_application_f = (float)RAM_Used_application_u / 1024 / 1024;
		RAM_Used_applet_f = (float)RAM_Used_applet_u / 1024 / 1024;
		RAM_Used_system_f = (float)RAM_Used_system_u / 1024 / 1024;
		RAM_Used_systemunsafe_f = (float)RAM_Used_systemunsafe_u / 1024 / 1024;
		RAM_Used_all_f = RAM_Used_application_f + RAM_Used_applet_f + RAM_Used_system_f + RAM_Used_systemunsafe_f;
		snprintf(RAM_all_c, sizeof RAM_all_c, "%.0f/%.0fMB", RAM_Used_all_f, RAM_Total_all_f);
		snprintf(RAM_var_compressed_c, sizeof RAM_var_compressed_c, "%s@%.1f", RAM_all_c, RAM_Hz_f);
		
		///Thermal
		SoC_temperatureC = (float)SoC_temperaturemiliC / 1000;
		PCB_temperatureC = (float)PCB_temperaturemiliC / 1000;
		skin_temperatureC = (float)skin_temperaturemiliC / 1000;
		snprintf(skin_temperature_c, sizeof skin_temperature_c, "%2.1f\u00B0C/%2.1f\u00B0C/%2.1f\u00B0C", SoC_temperatureC, PCB_temperatureC, skin_temperatureC);
		Rotation_SpeedLevel_percent = Rotation_SpeedLevel_f * 100;
		snprintf(Rotation_SpeedLevel_c, sizeof Rotation_SpeedLevel_c, "%2.2f%s", Rotation_SpeedLevel_percent, "%");
		
		///FPS
		snprintf(FPS_c, sizeof FPS_c, "PFPS:"); //Pushed Frames Per Second
		snprintf(FPSavg_c, sizeof FPSavg_c, "FPS:"); //Frames Per Second calculated from averaged frametime 
		snprintf(FPS_compressed_c, sizeof FPS_compressed_c, "%s\n%s", FPS_c, FPSavg_c);
		snprintf(FPS_c, sizeof FPS_c, "%u", FPS);
		snprintf(FPSavg_c, sizeof FPSavg_c, "%2.2f", FPSavg);
		snprintf(FPS_var_compressed_c, sizeof FPS_compressed_c, "%s\n%s", FPS_c, FPSavg_c);

		if (GameRunning == true) snprintf(Variables, sizeof Variables, "%s\n%s\n%s\n%s\n%s\n%s", CPU_compressed_c, GPU_Load_c, RAM_var_compressed_c, skin_temperature_c, Rotation_SpeedLevel_c, FPS_var_compressed_c);
		else snprintf(Variables, sizeof Variables, "%s\n%s\n%s\n%s\n%s", CPU_compressed_c, GPU_Load_c, RAM_var_compressed_c, skin_temperature_c, Rotation_SpeedLevel_c);

	}
    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (keysHeld & KEY_LSTICK) {
			if (keysHeld & KEY_RSTICK) {
				CloseThreads();
				tsl::goBack();
				return true;
			}
		}
		return false;
    }
};

//Main Menu
class MainMenu : public tsl::Gui {
public:
    MainMenu() { }

    virtual tsl::elm::Element* createUI() override {
		auto rootFrame = new tsl::elm::OverlayFrame("状态监视器", "v0.5");
		auto list = new tsl::elm::List();
		
		auto Full = new tsl::elm::ListItem("完整");
		Full->setClickListener([](u64 keys) {
			if (keys & KEY_A) {
				StartThreads();
				TeslaFPS = 1;
				refreshrate = 1;
				tsl::hlp::requestForeground(false);
				tsl::changeTo<FullOverlay>();
				return true;
			}
			return false;
		});
		list->addItem(Full);
		auto Mini = new tsl::elm::ListItem("迷你");
		Mini->setClickListener([](u64 keys) {
			if (keys & KEY_A) {
				StartThreads();
				TeslaFPS = 1;
				refreshrate = 1;
				alphabackground = 0x0;
				tsl::hlp::requestForeground(false);
				tsl::changeTo<MiniOverlay>();
				FullMode = false;
				return true;
			}
			return false;
		});
		list->addItem(Mini);
		auto comFPS = new tsl::elm::ListItem("FPS计数器");
		comFPS->setClickListener([](u64 keys) {
			if (keys & KEY_A) {
				StartFPSCounterThread();
				TeslaFPS = 31;
				refreshrate = 31;
				alphabackground = 0x0;
				tsl::hlp::requestForeground(false);
				FullMode = false;
				tsl::changeTo<com_FPS>();
				return true;
			}
			return false;
		});
		list->addItem(comFPS);

		rootFrame->setContent(list);

		return rootFrame;
	}

	virtual void update() override {
		if (TeslaFPS != 60) {
			FullMode = true;
			tsl::hlp::requestForeground(true);
			TeslaFPS = 60;
			alphabackground = 0xD;
			refreshrate = 1;
			systemtickfrequency = 19200000;
		}
	}
    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		if (keysHeld & KEY_B) {
			tsl::goBack();
			return true;
		}
		return false;
    }
};

class MonitorOverlay : public tsl::Overlay {
public:

	virtual void initServices() override {
		//Initialize services
		SaltySD = CheckPort();
		smCheck = smInitialize();
		if (R_SUCCEEDED(smCheck)) {
			if (hosversionAtLeast(8,0,0)) clkrstCheck = clkrstInitialize();
			else pcvCheck = pcvInitialize();
			tsCheck = tsInitialize();
			if (hosversionAtLeast(5,0,0)) tcCheck = tcInitialize();
			fanCheck = fanInitialize();
			if (R_SUCCEEDED(fanCheck)) {
				if (hosversionAtLeast(7,0,0)) fanCheck = fanOpenController(&g_ICon, 0x3D000001);
				else fanCheck = fanOpenController(&g_ICon, 1);
			}
			nvCheck = nvInitialize();
			if (R_SUCCEEDED(nvCheck)) nvCheck = nvOpen(&fd, "/dev/nvhost-ctrl-gpu");
			if (SaltySD == true) {
					dmntchtCheck = dmntchtInitialize();
			}
		}
		Hinted = envIsSyscallHinted(0x6F);
		
		//Assign functions to core of choose
		threadCreate(&t6, CheckIfGameRunning, NULL, NULL, 0x1000, 0x38, -2);
		
		//Start assigned functions
		threadStart(&t6);
	}

	virtual void exitServices() override {
		threadexit2 = true;
		threadWaitForExit(&t6);
		
		//Exit services
		dmntchtExit();
		clkrstExit();
		pcvExit();
		tsExit();
		tcExit();
		fanControllerClose(&g_ICon);
		fanExit();
		nvClose(fd);
		nvExit();
		smExit();

		
		//Free threads
		threadClose(&t6);
	}

    virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
    virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainMenu>();  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
    }
};

// This function gets called on startup to create a new Overlay object
int main(int argc, char **argv) {
    return tsl::loop<MonitorOverlay>(argc, argv);
}

#pragma once

#pragma warning(disable:4702)
#pragma warning(disable:4100)
#pragma warning(disable:4201)
#pragma warning(disable:4511)
#pragma warning(disable:4663)
#pragma warning(disable:4018)
#pragma warning(disable:4245)
#pragma warning(disable:4995)

#if _MSC_VER >= 1400
//if don't use below, time_t is 64bit
#define _USE_32BIT_TIME_T
#endif

#include "../eterLib/StdAfx.h"
#include "../eterPythonLib/StdAfx.h"
#include "../gameLib/StdAfx.h"
#include "../scriptLib/StdAfx.h"
#include "../milesLib/StdAfx.h"
#include "../EffectLib/StdAfx.h"
#include "../PRTerrainLib/StdAfx.h"
#include "../SpeedTreeLib/StdAfx.h"
#ifndef __D3DRM_H__
#define __D3DRM_H__
#endif

#define _snprintf snprintf

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <dshow.h>
#include <qedit.h>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include "Locale.h"
#include <random>
#include <cstdio>
#include <filesystem>
#include <string>
#include <bitset>
#include <memory>
#include <array>
#include <chrono>
#include <cmath>
#include "GameType.h"
extern DWORD __DEFAULT_CODE_PAGE__;

#define APP_NAME	"Metin 2"

enum
{
	POINT_MAX_NUM = 255,	
	CHARACTER_NAME_MAX_LEN = 32,
	PLAYER_NAME_MAX_LEN = 16,
};

void initudp();
void initapp();
void initime();
void initsystemSetting();
void initchr();
void initchrmgr();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initSectionDisplayer();
void initServerStateChecker();
void initTrade();
void initMiniMap();
void initProfiler();
void initEvent();
void initeffect();
void initsnd();
void initeventmgr();
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
void initfly();
void initquest();
void initsafebox();
void initguild();
void initMessenger();

#include "StdAfx.h"

#include <chrono>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <future>
#include <numeric>
#include <shared_mutex>

#include "Version.h"
#include "resource.h"
#include "PythonApplication.h"

#include <cryptopp/cryptoppLibLink.h>
#include "../eterPack/EterPackManager.h"
#include "../eterLib/Util.h"
#include "../eterBase/CPostIt.h"

#include <il/il.h>
#ifdef _DEBUG
	#include <crtdbg.h>
#endif

bool __IS_TEST_SERVER_MODE__ = false;
extern bool SetDefaultCodePage(DWORD codePage);

extern "C" 
{
	FILE __iob_func[3] = { *stdin, *stdout, *stderr };
}

constexpr static const char* const kRequiredPythonLibFilenames[] =
{
	"UserDict.pyc",
	"__future__.pyc",
	"copy_reg.pyc",
	"linecache.pyc",
	"ntpath.pyc",
	"os.pyc",
	"site.pyc",
	"stat.pyc",
	"string.pyc",
	"traceback.pyc",
	"types.pyc",
	"\n",
};

static bool AreRequiredPythonLibrariesAvailable() 
{
	const std::filesystem::path pythonLibraryPath = "lib";

	std::vector<std::future<bool>> futures;
	for (const char* const* pCurrentFilename = kRequiredPythonLibFilenames; *pCurrentFilename != "\n"; ++pCurrentFilename) 
	{
		futures.push_back(std::async(std::launch::async, [pythonLibraryPath, pCurrentFilename]() {
			return std::filesystem::exists(pythonLibraryPath / *pCurrentFilename);
		}));
	}

	for (auto& fut : futures) 
	{
		if (!fut.get()) 
		{
			return false;
		}
	}

	return true;
}

std::shared_mutex registerMutex;

static void RegisterIndividualPack(const std::string& folderPath, const std::string& packName) 
{
	std::scoped_lock lock(registerMutex);
	std::string packPath = folderPath + "/" + packName;
	CEterPackManager::Instance().RegisterPack(packPath.c_str(), "");
}

static void RegisterAllPacks(const std::string& folderPath, const std::vector<std::string>& packs) 
{
	std::vector<std::future<void>> futures;

	for (const auto& packName : packs) 
	{
		futures.push_back(std::async(std::launch::async, RegisterIndividualPack, folderPath, packName));
	}

	for (auto& future : futures) 
	{
		future.get();
	}
}

static std::vector<std::string> LoadPackNames(const std::string& indexPath)
{
	std::ifstream indexFile(indexPath.c_str());
	std::vector<std::string> packs;
	std::string packName;

	if (indexFile.is_open())
	{
		while (std::getline(indexFile, packName))
		{
			packs.push_back(packName);
		}
	}

	return packs;
}

static bool PackInitialize(const char* folderPathCStr)
{
	std::string folderPath(folderPathCStr);
	std::string indexPath = folderPath + "/Index";

	auto packs = LoadPackNames(indexPath);

	if (packs.empty())
	{
		return false;
	}

	std::vector<std::future<void>> futures;
	unsigned int maxThreads = std::thread::hardware_concurrency();

	for (const auto& packName : packs) 
	{
		if (futures.size() >= maxThreads) 
		{
			futures.front().get();
			futures.erase(futures.begin());
		}

		futures.emplace_back(std::async(std::launch::async, RegisterIndividualPack, folderPath, packName));
	}

	for (auto& future : futures) 
	{
		future.get();
	}

	CEterPackManager::Instance().RegisterRootPack((folderPath + "/core").c_str());
	CTextFileLoader::SetCacheMode();
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(true);
	CSoundData::SetPackMode();

	return true;
}

static void InitializeAllModules()
{
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();
	initudp();
	initapp();
	initsystemSetting();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initProfiler();
	initEvent();
	initeffect();
	initfly();
	initsnd();
	initeventmgr();
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
	initguild();
	initServerStateChecker();
}

static bool RunMainScript(CPythonLauncher& pyLauncher, const std::string& lpCmdLine)
{
	InitializeAllModules();

	PyObject* builtins = PyImport_ImportModule("__builtin__");

#ifdef _DISTRIBUTE
	PyModule_AddIntConstant(builtins, "__DEBUG__", 0);
#else
	PyModule_AddIntConstant(builtins, "__DEBUG__", 1);
#endif

	if (!pyLauncher.RunFile("system.py"))
	{
		TraceError("RunMain Error");
		return false;
	}

	return true;
}

static bool InitializeThreadsAndModules()
{
	std::thread initThread([]()
	{
		static CLZO lzo;
		static CEterPackManager EterPackManager;
	});

	initThread.join();
	return true;
}

static bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	ilInit();

	if (!InitializeThreadsAndModules())
	{
		LogBox("Initialization of threads and modules failed.");
		return false;
	}

#ifdef _DEBUG
	OpenConsoleWindow();
	OpenLogFile(true);
#else
	OpenLogFile(false);
#endif

	if (!PackInitialize("pack"))
	{
		LogBox("Pack Initialization failed. Check log.txt file.");
		return false;
	}

	auto app = std::make_unique<CPythonApplication>();
	if (!app->Initialize(hInstance)) 
	{
		LogBox("CPythonApplication Initialization failed. Check log.txt file.");
		return false;
	}

	CPythonLauncher pyLauncher;
	bool ret = pyLauncher.Create() && RunMainScript(pyLauncher, lpCmdLine);

	app->Destroy();
	return ret;
}

int32_t APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int32_t nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if (!AreRequiredPythonLibrariesAvailable())
	{
		LogBoxf("Fatal Error: Required Python library files are missing. Application will terminate.");
		return EXIT_FAILURE;
	}

	LocaleService_LoadConfig("config/locale.cfg");
	SetDefaultCodePage(LocaleService_GetCodePage());

	try
	{
		if (!Main(hInstance, lpCmdLine))
		{
			return EXIT_FAILURE;
		}
	}
	catch (const std::exception& e)
	{
		LogBoxf(std::string("Exception caught: ").append(e.what()).c_str());
		return EXIT_FAILURE;
	}

	::CoUninitialize();

	return EXIT_SUCCESS;
}
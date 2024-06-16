#include "StdAfx.h"
#include "../eterBase/Error.h"
#include "../eterlib/Camera.h"
#include "../eterlib/AttributeInstance.h"
#include "../gamelib/AreaTerrain.h"
#include "../EterGrnLib/Material.h"
#include "../CWebBrowser/CWebBrowser.h"

#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

#include "ProcessScanner.h"

#include "NProtectGameGuard.h"
#include "CheckLatestFiles.h"

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MinFogDistance = 2400.0f;
double SpecularSpeed = 0.007f;

constexpr float DefaultCameraRotateSpeed = 1.5f;
constexpr float DefaultCameraPitchSpeed = 1.5f;
constexpr float DefaultCameraZoomSpeed = 0.05f;

CPythonApplication* CPythonApplication::ms_pInstance;

CPythonApplication::CPythonApplication()
	: m_poMouseHandler(nullptr),
	m_fAveRenderTime(0.0F),
	m_fGlobalTime(0.0F),
	m_fGlobalElapsedTime(0.0F),
	m_dwUpdateFPS(0),
	m_dwRenderFPS(0),
	m_dwFaceCount(0),
	m_dwLButtonDownTime(0),
	m_bCursorVisible(true),
	m_bLiarCursorOn(false),
	m_iCursorMode(CURSOR_MODE_HARDWARE),
	m_isWindowed(false),
	m_isFrameSkipDisabled(false)
{
	CTimer::Instance().UseCustomTime();

	m_dwWidth = 800;
	m_dwHeight = 600;

	ms_pInstance = this;
	m_isWindowFullScreenEnabled = false;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd = 0.0f;

	m_dwFaceAccCount = 0;
	m_dwFaceAccTime = 0;

	m_dwFaceSpdSum = 0;
	m_dwFaceSpdCount = 0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = false;
	m_fCameraRotateSpeed = DefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = DefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = DefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MinFogDistance = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
	{
		m_isFrameSkipDisabled = false;
	}
	else
	{
		m_isFrameSkipDisabled = true;
	}
}

void CPythonApplication::GetInfo(uint32_t eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
		case INFO_ACTOR:
			m_kChrMgr.GetInfo(pstInfo);
			break;
		case INFO_EFFECT:
			m_kEftMgr.GetInfo(pstInfo);			
			break;
		case INFO_ITEM:
			m_pyItem.GetInfo(pstInfo);
			break;
		case INFO_TEXTTAIL:
			m_pyTextTail.GetInfo(pstInfo);
			break;
	}
}

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PyThreadState* ts = PyThreadState_Get();
	PyFrameObject* frame = ts->frame;

	while (frame != 0)
	{
		char const* filename = PyString_AsString(frame->f_code->co_filename);
		char const* name = PyString_AsString(frame->f_code->co_name);
		TraceError("filename = %s, name = %s", filename, name);
		frame = frame->f_back;
	}

	PostQuitMessage(0);
}

void CPythonApplication::Exit()
{
	PostQuitMessage(0);
}

void CPythonApplication::RenderGame()
{	const float perspectiveAngle = 30.0f;
	float aspect = m_kWndMgr.GetAspect();
	float farClip = m_pyBackground.GetFarClip();
	m_pyGraphic.SetPerspective(perspectiveAngle, aspect, 100.0, farClip);

	PreRenderOperations();
	RenderEnvironmentAndCharacters();
	return;
}

void CPythonApplication::PreRenderOperations()
{
	CCullingManager::Instance().Process();
	m_kChrMgr.Deform();
	m_pyBackground.RenderCharacterShadowToTexture();
	m_pyGraphic.SetGameRenderState();
	m_pyGraphic.PushState();

	long lx, ly;
	m_kWndMgr.GetMousePosition(lx, ly);
	m_pyGraphic.SetCursorPosition(lx, ly);
}

void CPythonApplication::RenderEnvironmentAndCharacters()
{
	m_pyBackground.RenderSky();
	m_pyBackground.RenderBeforeLensFlare();
	m_pyBackground.RenderCloud();
	m_pyBackground.BeginEnvironment();
	m_pyBackground.Render();
	m_pyBackground.SetCharacterDirLight();
	m_kChrMgr.Render();
	m_pyBackground.SetBackgroundDirLight();
	m_pyBackground.RenderWater();
	m_pyBackground.RenderSnow();
	m_pyBackground.RenderEffect();
	m_pyBackground.EndEnvironment();
	m_kEftMgr.Render();
	m_pyItem.Render();
	m_FlyingManager.Render();
	m_pyBackground.BeginEnvironment();
	m_pyBackground.RenderPCBlocker();
	m_pyBackground.EndEnvironment();
	m_pyBackground.RenderAfterLensFlare();
}

void CPythonApplication::UpdateGame()
{
	POINT ptMouse;
	GetMousePosition(&ptMouse);
	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);

	UpdateScreenSettings();

	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	UpdateManagers(ptMouse);

	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
}

void CPythonApplication::UpdateScreenSettings()
{
	CScreen s;
	float fAspect = UI::CWindowManager::Instance().GetAspect();
	float fFarClip = CPythonBackground::Instance().GetFarClip();

	s.SetPerspective(30.0f, fAspect, 100.0f, fFarClip);
	s.BuildViewFrustum();
}

void CPythonApplication::UpdateManagers(const POINT& ptMouse)
{
	m_kChrMgr.Update();
	m_kEftMgr.Update();
	m_kEftMgr.UpdateSound();
	m_FlyingManager.Update();
	m_pyItem.Update(ptMouse);
	m_pyPlayer.Update();
}

void CPythonApplication::SkipRenderBuffering(uint32_t dwSleepMSec)
{
	m_dwBufSleepSkipTime = ELTimer_GetMSec() + dwSleepMSec;
}

bool CPythonApplication::Process()
{
#if defined(CHECK_LATEST_DATA_FILES)
	if (CheckLatestFiles_PollEvent())
		return false;
#endif

	ELTimer_SetFrameMSec();

	// 	m_Profiler.Clear();
	DWORD dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static DWORD	s_dwUpdateFrameCount = 0;
	static DWORD	s_dwRenderFrameCount = 0;
	static DWORD	s_dwFaceCount = 0;
	static UINT		s_uiLoad = 0;
	static DWORD	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS		= s_dwUpdateFrameCount;
		m_dwRenderFPS		= s_dwRenderFrameCount;
		m_dwLoad			= s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / max(1, s_dwRenderFrameCount);

		s_dwCheckTime		= ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
	}

	// Update Time
	static BOOL s_bFrameSkip = false;
	static UINT s_uiNextFrameTime = ELTimer_GetMSec();

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime1=ELTimer_GetMSec();
#endif
	CTimer& rkTimer=CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	UINT uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;	//17 - 1ĂĘ´ç 60fps±âÁŘ.

	DWORD updatestart = ELTimer_GetMSec();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime2=ELTimer_GetMSec();
#endif
	// Network I/O	
	m_pyNetworkStream.Process();	
	//m_pyNetworkDatagram.Process();

	m_kGuildMarkUploader.Process();

#ifdef USE_NPROTECT_GAMEGUARD
	if (GameGuard_IsError())
		return false;
#endif

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();

#ifdef __PERFORMANCE_CHECK__		
	DWORD dwUpdateTime3=ELTimer_GetMSec();
#endif
	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime4=ELTimer_GetMSec();
#endif
	// Mouse
	POINT Point;
	if (GetCursorPos(&Point))
	{
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);		
	}
	//////////////////////
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime5=ELTimer_GetMSec();
#endif

	__UpdateCamera();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime6=ELTimer_GetMSec();
#endif
	// Update Game Playing
	CResourceManager::Instance().Update();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime7=ELTimer_GetMSec();
#endif
	OnCameraUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime8=ELTimer_GetMSec();
#endif
	OnMouseUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime9=ELTimer_GetMSec();
#endif
	OnUIUpdate();

#ifdef __PERFORMANCE_CHECK__		
	DWORD dwUpdateTime10=ELTimer_GetMSec();

	if (dwUpdateTime10-dwUpdateTime1>10)
	{			
		static FILE* fp=fopen("perf_app_update.txt", "w");

		fprintf(fp, "AU.Total %d (Time %d)\n", dwUpdateTime9-dwUpdateTime1, ELTimer_GetMSec());
		fprintf(fp, "AU.TU %d\n", dwUpdateTime2-dwUpdateTime1);
		fprintf(fp, "AU.NU %d\n", dwUpdateTime3-dwUpdateTime2);
		fprintf(fp, "AU.KU %d\n", dwUpdateTime4-dwUpdateTime3);
		fprintf(fp, "AU.MP %d\n", dwUpdateTime5-dwUpdateTime4);
		fprintf(fp, "AU.CP %d\n", dwUpdateTime6-dwUpdateTime5);
		fprintf(fp, "AU.RU %d\n", dwUpdateTime7-dwUpdateTime6);
		fprintf(fp, "AU.CU %d\n", dwUpdateTime8-dwUpdateTime7);
		fprintf(fp, "AU.MU %d\n", dwUpdateTime9-dwUpdateTime8);
		fprintf(fp, "AU.UU %d\n", dwUpdateTime10-dwUpdateTime9);			
		fprintf(fp, "----------------------------------\n");
		fflush(fp);
	}		
#endif

	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	DWORD dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = ((float)dt / (float)uiFrameTime) * uiFrameTime; 

		if ( dt >= 500 )
		{
			s_uiNextFrameTime += nAdjustTime; 
			printf("FrameSkip ş¸Á¤ %d\n",nAdjustTime);
			CTimer::Instance().Adjust(nAdjustTime);
		}

		s_bFrameSkip = true;
		bCurrentLateUpdate = TRUE;
	}

	if (m_isFrameSkipDisabled)
		s_bFrameSkip = false;

	if (!s_bFrameSkip)
	{
		CGrannyMaterial::TranslateSpecularMatrix(SpecularSpeed, SpecularSpeed, 0.0f);

		DWORD dwRenderStartTime = ELTimer_GetMSec();		

		bool canRender = true;

		if (m_isMinimizedWnd)
		{
			canRender = false;
		}
		else
		{
			if (m_pyGraphic.IsLostDevice())
			{
				CPythonBackground& rkBG = CPythonBackground::Instance();
				rkBG.ReleaseCharacterShadowTexture();
				if (m_pyGraphic.RestoreDevice())					
				{
					rkBG.CreateCharacterShadowTexture();
				}
				else
					canRender = false;			
			}
		}

		if (!IsActive())
		{
			SkipRenderBuffering(3000);
		}

		if (!canRender)
		{
			SkipRenderBuffering(3000);
		}
		else
		{
			// RestoreLostDevice
			CCullingManager::Instance().Update();
			if (m_pyGraphic.Begin())
			{
				m_pyGraphic.ClearDepthBuffer();
#ifdef _DEBUG
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				// Interface
				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();

				m_pyGraphic.End();
				m_pyGraphic.Show();

				DWORD dwRenderEndTime = ELTimer_GetMSec();

				static DWORD s_dwRenderCheckTime = dwRenderEndTime;
				static DWORD s_dwRenderRangeTime = 0;
				static DWORD s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;			
				s_dwRenderRangeTime += m_dwCurRenderTime;				
				++s_dwRenderRangeFrame;

				if (dwRenderEndTime-s_dwRenderCheckTime>1000)
				{
					m_fAveRenderTime=float(double(s_dwRenderRangeTime)/double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime=ELTimer_GetMSec();
					s_dwRenderRangeTime=0;
					s_dwRenderRangeFrame=0;
				}										

				DWORD dwCurFaceCount=m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{	
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						DWORD dwBufRenderTime = s_fBufRenderTime;

						if (m_isWindowed)
						{						
							if (dwBufRenderTime>58)
								dwBufRenderTime=64;
							else if (dwBufRenderTime>42)
								dwBufRenderTime=48;
							else if (dwBufRenderTime>26)
								dwBufRenderTime=32;
							else if (dwBufRenderTime>10)
								dwBufRenderTime=16;
							else
								dwBufRenderTime=8;
						}	

						m_fAveRenderTime=s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd=(m_dwFaceAccCount/m_dwFaceAccTime);

					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio=0.3f;
						s_fAveRenderTime=(s_fAveRenderTime*(100.0f-fRatio)+max(16.0f, m_dwCurRenderTime)*fRatio)/100.0f;


						float fFar=25600.0f;
						float fNear= MinFogDistance;
						double dbAvePow=double(1000.0f/s_fAveRenderTime);
						double dbMaxPow=60.0;
						float fDistance=max(fNear+(fFar-fNear)*(dbAvePow)/dbMaxPow, fNear);
						m_pyBackground.SetViewDistanceSet(0, fDistance);
					}
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
				}
				else
				{
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

	int rest = s_uiNextFrameTime - ELTimer_GetMSec();

	if (rest > 0 && !bCurrentLateUpdate )
	{
		s_uiLoad -= rest;
		Sleep(rest);
	}	

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{	
	m_poMouseHandler = poMouseHandler;
}

int CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
		// µđąŮŔĚ˝ş°ˇ ľřŔ¸¸é ÇÁ·Î±×·ĄŔĚ Áľ·á µÇľîľß ÇŃ´Ů.
	case CGraphicDevice::DEVICESTATE_NULL:
		return DEVICE_STATE_FALSE;

		// DEVICESTATE_BROKENŔĎ ¶§´Â ´ŮŔ˝ ·çÇÁżˇĽ­ şą±¸ µÉ Ľö ŔÖµµ·Ď ¸®ĹĎ ÇŃ´Ů.
		// ±×łÉ ÁřÇŕÇŇ °ćżě DrawPrimitive °°Ŕş °ÍŔ» ÇĎ¸é ÇÁ·Î±×·ĄŔĚ ĹÍÁř´Ů.
	case CGraphicDevice::DEVICESTATE_BROKEN:
		return DEVICE_STATE_SKIP;

	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		if (!m_grpDevice.Reset())
			return DEVICE_STATE_SKIP;

		break;
	}

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)
{
	int iRet;

	m_grpDevice.InitBackBufferCount(2);

	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit,frequency);

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:
		//PyErr_SetString(PyExc_RuntimeError, "DirectX 8.1 or greater required to run game");
		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 8.1 or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:
		//PyErr_SetString(PyExc_RuntimeError, "GraphicDevice create failed");
		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_FORMAT:
		SET_EXCEPTION(CREATE_FORMAT);
		TraceError("CreateDevice: Change the screen format");
		return false;

		/*case CGraphicDevice::CREATE_GET_ADAPTER_DISPLAY_MODE:
		//PyErr_SetString(PyExc_RuntimeError, "GetAdapterDisplayMode failed");
		SET_EXCEPTION(CREATE_GET_ADAPTER_DISPLAY_MODE);
		TraceError("CreateDevice: GetAdapterDisplayMode failed");
		return false;*/

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
			{
				CGrannyLODController::SetMinLODMode(true);
			}
			return true;
		}

		//PyErr_SetString(PyExc_RuntimeError, "Unknown Error!");
		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::Loop()
{	
	while (1)
	{	
		if (IsMessage())
		{
			if (!MessageProcess())
				break;
		}
		else
		{
			if (!Process())
				break;

			m_dwLastIdleTime=ELTimer_GetMSec();
		}
	}
}

// SUPPORT_NEW_KOREA_SERVER
bool LoadLocaleData(const char* localePath)
{
	CPythonNonPlayer&	rkNPCMgr	= CPythonNonPlayer::Instance();

	CItemManager&		rkItemMgr	= CItemManager::Instance();	
	CPythonSkill&		rkSkillMgr	= CPythonSkill::Instance();
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	PythonLocaleManager& rkLocale = PythonLocaleManager::Instance();
	CPythonMiniMap& rkAtlasLoader = CPythonMiniMap::Instance();

	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];	
	char szLocaleTextString[256];
	char szLocaleQuestString[256];
	char szLocaleQuizString[256];	
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];
	char szTipList[256];
	char szTip[256];


	snprintf(szItemList,	sizeof(szItemList) ,	"%s/item_list.txt",	localePath);		
	snprintf(szItemProto,	sizeof(szItemProto),	"%s/item_proto",	localePath);
	snprintf(szItemDesc,	sizeof(szItemDesc),	"%s/itemdesc.txt",	localePath);	
	snprintf(szLocaleTextString, sizeof(szLocaleTextString), "%s/locale_string.txt", localePath);
	snprintf(szLocaleQuestString, sizeof(szLocaleQuestString), "%s/locale_quest.txt", localePath);
	snprintf(szLocaleQuizString, sizeof(szLocaleQuizString), "%s/locale_quiz.txt", localePath);	
	snprintf(szMobProto,	sizeof(szMobProto),	"%s/mob_proto",		localePath);	
	snprintf(szSkillDescFileName, sizeof(szSkillDescFileName),	"%s/SkillDesc.txt", localePath);
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName),	"%s/SkillTable.txt", localePath);	
	snprintf(szInsultList,	sizeof(szInsultList),	"%s/insult.txt", localePath);
	snprintf(szTipList, sizeof(szTipList), "%s/loading_tip_list.txt", localePath);
	snprintf(szTip, sizeof(szTip), "%s/loading_tip_vnum.txt", localePath);
	std::string configPath = std::string(localePath) + "/disabled_atlas.json";

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();	
	rkSkillMgr.Destroy();

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}	

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);	
	}
	
	if (!rkLocale.LoadGeneralLocaleStrings(szLocaleTextString))
	{
		TraceError("LoadLocaleData - LoadLocaleString(%s) Error", szLocaleTextString);
		return false;
	}

	if (!rkLocale.LoadQuestLocaleStrings(szLocaleQuestString))
	{
		TraceError("LoadLocaleData - LoadQuestLocaleString(%s) Error", szLocaleQuestString);
		return false;
	}

	if (!rkLocale.LoadOXQuizLocaleStrings(szLocaleQuizString))
	{
		TraceError("LoadLocaleData - LoadOXQuizLocaleString(%s) Error", szLocaleQuizString);
		return false;
	}	

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szMobProto);
		return false;
	}

	if (!rkNetStream.LoadInsultList(szInsultList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);				
	}

	if (rkNetStream.TipManager == nullptr)
	{
		auto tempTipManager = std::make_unique< CPythonNetworkStream::CTIPMANAGER >();

		if (!tempTipManager->InitList(szTipList))
		{
			TraceError("LoadLocaleData - LoadTipList(%s) Error", szTipList);
			return false;
		}

		if (!tempTipManager->InitVnum(szTip))
		{
			TraceError("LoadLocaleData - LoadTipVnum(%s) Error", szTip);
			return false;
		}

		rkNetStream.TipManager = std::move(tempTipManager);
	}

	if (LocaleService_IsYMIR())
	{	
		char szEmpireTextConvFile[256];
		for (DWORD dwEmpireID=1; dwEmpireID<=3; ++dwEmpireID)
		{			
			sprintf(szEmpireTextConvFile, "%s/lang%d.cvt", localePath, dwEmpireID);
			if (!rkNetStream.LoadConvertTable(dwEmpireID, szEmpireTextConvFile))
			{
				TraceError("LoadLocaleData - CPythonNetworkStream::LoadConvertTable(%d, %s) FAILURE", dwEmpireID, szEmpireTextConvFile);			
			}
		}
	}

	if (!rkAtlasLoader.LoadDisabledAtlasConfig(configPath))
	{
		TraceError("Unable to load disabled atlas configuration from: %s", configPath.c_str());
	}

	return true;
}
// END_OF_SUPPORT_NEW_KOREA_SERVER

unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION |   WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject* poSelf, const char* c_szName, int width, int height, int Windowed)
{
	NANOBEGIN
		Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	bool bAnotherWindow = false;

	if (FindWindow(NULL, c_szName))
		bAnotherWindow = true;

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	UINT WindowMode = __GetWindowMode(Windowed ? true : false);

	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_METIN2)), IDC_CURSOR_NORMAL))
	{
		//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Create failed");
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

#ifdef USE_NPROTECT_GAMEGUARD
	if (!GameGuard_Run(CMSWindow::GetWindowHandle()))
		return false;
#endif
#ifdef XTRAP_CLIENT_ENABLE
	if (!XTrap_CheckInit())
		return false;
#endif

	if (m_pySystem.IsUseDefaultIME())
	{
		CPythonIME::Instance().UseDefaultIME();
	}

	const auto windowHandle = GetWindowHandle();

	if (!m_pySystem.IsWindowed())
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnabled = true;
		SetFullScreenWindow(windowHandle, width, height, m_pySystem.GetBPP());
		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		m_isWindowed = Windowed;

		if (m_isWindowed)
		{
			RECT workArea{};
			SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

			const uint32_t workAreaWidth = workArea.right - workArea.left;
			const uint32_t workAreaHeight = workArea.bottom - workArea.top;

			const uint32_t windowWidth = CalculateWindowWidth(m_pySystem.GetWidth());
			const uint32_t windowHeight = CalculateWindowHeight(m_pySystem.GetHeight());

			const uint32_t x = workAreaWidth / 2 - windowWidth / 2;
			const uint32_t y = workAreaHeight / 2 - windowHeight / 2;

			SetPosition(x, y);
		}
		else
		{
			SetPosition(0, 0);
		}
	}

	NANOEND
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Cursor
		if (!CreateCursors())
		{
			//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Cursors Create Error");
			TraceError("CMSWindow::Cursors Create Error");
			SET_EXCEPTION("CREATE_CURSOR");
			return false;
		}

	if (!m_pySystem.IsNoSoundCard())
	{
		// Sound
		if (!m_SoundManager.Create())
		{
			// NOTE : 중국측의 요청으로 생략
			//		LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_SOUND_DEVICE));
		}
	}

	extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

	if (!m_pySystem.IsAutoTiling())
		GRAPHICS_CAPS_SOFTWARE_TILING = m_pySystem.IsSoftwareTiling();

	// Device
	if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
		return false;

	GrannyCreateSharedDeformBuffer();

	if (m_pySystem.IsAutoTiling())
	{
		if (m_grpDevice.IsFastTNL())
		{
			m_pyBackground.ReserveSoftwareTilingEnable(false);
		}
		else
		{
			m_pyBackground.ReserveSoftwareTilingEnable(true);
		}
	}
	else
	{
		m_pyBackground.ReserveSoftwareTilingEnable(m_pySystem.IsSoftwareTiling());
	}

	SetVisibleMode(true);

	if (m_isWindowFullScreenEnabled) //m_pySystem.IsUseDefaultIME() && !m_pySystem.IsWindowed())
	{
		SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
	}

	if (!InitializeKeyboard(GetWindowHandle()))
		return false;

	m_pySystem.GetDisplaySettings();

	// Mouse
	if (m_pySystem.IsSoftwareCursor())
		SetCursorMode(CURSOR_MODE_SOFTWARE);
	else
		SetCursorMode(CURSOR_MODE_HARDWARE);

	// Network
	if (!m_netDevice.Create())
	{
		//PyErr_SetString(PyExc_RuntimeError, "NetDevice::Create failed");
		TraceError("NetDevice::Create failed");
		SET_EXCEPTION("CREATE_NETWORK");
		return false;
	}

	if (!m_grpDevice.IsFastTNL())
		CGrannyLODController::SetMinLODMode(true);

	m_pyItem.Create();

	// Other Modules
	DefaultFont_Startup();

	CPythonIME::Instance().Create(GetWindowHandle());
	CPythonIME::Instance().SetText("", 0);
	CPythonTextTail::Instance().Initialize();

	// Light Manager
	m_LightManager.Initialize();

	CGraphicImageInstance::CreateSystem(32);

	// 백업
	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);
	m_dwStickyKeysFlag = sStickKeys.dwFlags;

	// 설정
	sStickKeys.dwFlags &= ~(SKF_AVAILABLE | SKF_HOTKEYACTIVE);
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);

	// SphereMap
	CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
	CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
	return true;
}

void CPythonApplication::SetGlobalCenterPosition(LONG x, LONG y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}


void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime	= ELTimer_GetMSec();
	m_tServerTime		= tTime;
	m_tLocalStartTime	= time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

// 2005.03.28 - MALL ľĆŔĚĹŰżˇ µéľîŔÖ´Â ˝Ă°ŁŔÇ ´ÜŔ§°ˇ Ľ­ąöżˇĽ­ time(0) Ŕ¸·Î ¸¸µéľîÁö´Â
//              °ŞŔĚ±â ¶§ą®żˇ ´ÜŔ§¸¦ ¸ÂĂß±â Ŕ§ÇŘ ˝Ă°Ł °ü·Ă Ăł¸®¸¦ ş°µµ·Î Ăß°ˇ
time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int & riPort)
{
	rstIP	= m_strIP;
	riPort	= m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = DefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = DefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = DefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::SetTitle(const char* szTitle)
{
	CMSWindow::SetText(szTitle);
}

uint32_t CPythonApplication::CalculateWindowWidth(uint32_t clientWidth)
{
	return clientWidth + GetSystemMetrics(SM_CXBORDER) * 2 + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CXFRAME) * 2;
}

uint32_t CPythonApplication::CalculateWindowHeight(uint32_t clientHeight)
{
	return clientHeight + GetSystemMetrics(SM_CYBORDER) * 2 + GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

void CPythonApplication::Destroy()
{
	WebBrowser_Destroy();

	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

	DestroyCollisionInstanceSystem();

	m_pySystem.SaveInterfaceStatus();

	m_pyEventManager.Destroy();	
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyChat.Destroy();	
	m_kChrMgr.Destroy();
	m_pyTextTail.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();
	
	//m_pyNetworkDatagram.Destroy();	

	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();


	m_SoundManager.Destroy();
	m_grpDevice.Destroy();

	// FIXME : ¸¸µéľîÁ® ŔÖÁö ľĘŔ˝ - [levites]
	//CSpeedTreeForestDirectX8::Instance().Clear();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
}

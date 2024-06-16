#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
		enum EWindow
		{
			WINDOW_STATUS	= 0,
			WINDOW_INVENTORY = 1,
			WINDOW_ABILITY	= 2,
			WINDOW_SOCIETY	= 3,
			WINDOW_JOURNAL	= 4,
			WINDOW_COMMAND	= 5,
			WINDOW_QUICK	= 6,
			WINDOW_GAUGE	= 7,
			WINDOW_MINIMAP	= 8,
			WINDOW_CHAT		= 9,
			WINDOW_MAX_NUM	= 10,
		};

		enum ESystem
		{
			FREQUENCY_MAX_NUM  = 30,
			RESOLUTION_MAX_NUM = 100,
		};

		typedef struct SResolution
		{
			DWORD	width;
			DWORD	height;
			DWORD	bpp;		// bits per pixel (high-color = 16bpp, true-color = 32bpp)

			DWORD	frequency[20];
			BYTE	frequency_count;
		} TResolution;

		typedef struct SWindowStatus
		{
			int isVisible;
			int isMinimized;
			int	xPosition;
			int yPosition;
			int	height;
		} TWindowStatus;

		typedef struct SConfig
		{
			DWORD width;
			DWORD height;
			DWORD bpp;
			DWORD frequency;

			bool is_software_cursor;
			bool is_object_culling;
			int iDistance;
			int iShadowLevel;

			float music_volume;
			BYTE voice_volume;

			int gamma;

			int isSaveID;
			char SaveID[20];

			bool bWindowed;
			bool bDecompressDDS;
			bool bNoSoundCard;
			bool bUseDefaultIME;
			uint8_t bSoftwareTiling;
			bool bViewChat;
			bool bAlwaysShowName;
			bool bShowDamage;
			bool bShowSalesText;
			
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
			bool bNightMode;
			bool bSnowMode;
			bool bSnowTextureMode;
#endif
#if defined(WJ_SHOW_MOB_INFO)
			#if defined(ENABLE_SHOW_MOBAIFLAG)
			bool bShowMobAIFlag;
			#endif
			#if defined(ENABLE_SHOW_MOBLEVEL)
			bool bShowMobLevel;
			#endif
#endif
			bool bIsFogModeEnabled;
			uint8_t bCameraMode;
		} TConfig;

	public:
		CPythonSystem();
		virtual ~CPythonSystem();

		void Clear();
		void SetInterfaceHandler(PyObject * poHandler);
		void DestroyInterfaceHandler();

		// Config
		void							SetDefaultConfig();
		bool							LoadConfig();
		bool							SaveConfig();
		void							ApplyConfig();
		void							SetConfig(TConfig * set_config);
		TConfig *						GetConfig();
		void							ChangeSystem();

		// Interface
		bool							LoadInterfaceStatus();
		void							SaveInterfaceStatus();
		bool							isInterfaceConfig();
		const TWindowStatus &			GetWindowStatusReference(int iIndex);

		DWORD							GetWidth();
		DWORD							GetHeight();
		DWORD							GetBPP();
		DWORD							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		bool							IsAutoTiling();
		bool							IsSoftwareTiling();
		void							SetSoftwareTiling(bool isEnable);
		void							SetViewChatFlag(int iFlag);
		void							SetAlwaysShowNameFlag(int iFlag);
		void							SetShowDamageFlag(int iFlag);
		void							SetShowSalesTextFlag(int iFlag);

#if defined(WJ_SHOW_MOB_INFO)
		#if defined(ENABLE_SHOW_MOBAIFLAG)
		bool IsShowMobAIFlag() const;
		void SetShowMobAIFlagFlag(bool iFlag);
		#endif
		#if defined(ENABLE_SHOW_MOBLEVEL)
		bool IsShowMobLevel() const;
		void SetShowMobLevelFlag(bool iFlag);
		#endif
#endif

		void SetCameraMode(uint8_t bMode);
		uint8_t GetCameraMode() const;

		// Window
		void SaveWindowStatus(int index, int isVisible, int isMinimized, int xPos, int yPos, int height);

		// SaveID
		int								IsSaveID();
		const char *					GetSaveID();
		void							SetSaveID(int iValue, const char * c_szSaveID);

		/// Display
		void							GetDisplaySettings();

		int								GetResolutionCount();
		int								GetFrequencyCount(int index);
		bool							GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp);
		bool							GetFrequency(int index, int freq_index, OUT DWORD *frequncy);
		int								GetResolutionIndex(DWORD width, DWORD height, DWORD bpp);
		int								GetFrequencyIndex(int res_index, DWORD frequency);
		bool							isViewCulling();

		float GetMusicVolume();
		int GetSoundVolume();
		void SetMusicVolume(float fVolume);
		void SetSoundVolumef (float fVolume);

		int								GetDistance();
		int								GetShadowLevel();
		void							SetShadowLevel(unsigned int level);

		void							SetFogMode(bool bEnable);
		bool							GetFogMode() const;
		
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
		void SetNightModeOption(bool bEnable);
		bool GetNightModeOption() const;
		
		void SetSnowModeOption(bool bEnable);
		bool GetSnowModeOption() const;

		void SetSnowTextureModeOption(bool bEnable);
		bool GetSnowTextureModeOption() const;
#endif		

	protected:
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

		bool							m_isInterfaceConfig;
		PyObject *						m_poInterfaceHandler;
		TWindowStatus					m_WindowStatus[WINDOW_MAX_NUM];
};
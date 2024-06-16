import ui
import net
import app
import dbg
import player
import wndMgr
import chrmgr
import uiChat
import uiHelp
import uiShop
import emotion
import uiRefine
import uiSystem
import uiOption
import uiWhisper
import uiRestart
import uiToolTip
import colorInfo
import constInfo
import uiExchange
import localeInfo
import background
import uiPickMoney
import uiMessenger
import uiPointReset
import uiAttachMetin
import uiScriptLocale
import stringCommander
import playerSettingModule

class LoadingWindow(ui.ScriptWindow):
	def __init__(self, stream):
		ui.Window.__init__(self)
		net.SetPhaseWindow(net.PHASE_WINDOW_LOAD, self)
		
		self.stream = stream
		self.loadingImage = None
		self.loadingGage = None
		self.errMsg = None
		self.update = None
		self.playerX = None
		self.playerY = None
		self.loadStepList = []

	def __del__(self):
		net.SetPhaseWindow(net.PHASE_WINDOW_LOAD, 0)
		ui.Window.__del__(self)

	def Open(self):
		self.load_script_file()
		self.initialize_ui_elements()
		self.errMsg.Hide()
		self.loadingTip.SetText(net.GetTipInfo())
		self.configure_loading_image()
		
	def load_script_file(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, uiScriptLocale.LOCALE_UISCRIPT_PATH + "LoadingWindow.py")
		except:
			exception.Abort("LoadingWindow.Open - LoadScriptFile Error")		

	def initialize_ui_elements(self):
		try:
			self.loadingImage = self.GetChild("BackGround")
			self.errMsg = self.GetChild("ErrorMessage")
			self.loadingGage = self.GetChild("FullGage")
			self.loadingInfo = self.GetChild("LoadingInfo_Text")
			self.loadingTip = self.GetChild("LoadingTip")
		except:
			exception.Abort("LoadingWindow.Open - GetChild Error")
			
	def configure_loading_image(self):	
		imgFileNameDict = {
			0 : uiScriptLocale.LOCALE_UISCRIPT_PATH + "loading/loading0.sub",
			1 : uiScriptLocale.LOCALE_UISCRIPT_PATH + "loading/loading1.sub",
			2 : uiScriptLocale.LOCALE_UISCRIPT_PATH + "loading/loading2.sub",
			3 : uiScriptLocale.LOCALE_UISCRIPT_PATH + "loading/loading3.sub",
		}	
	
		imgFileName = None
		try:
			imgFileName = imgFileNameDict[app.GetRandom(0, len(imgFileNameDict) - 1)]
			self.loadingImage.LoadImage(imgFileName)

		except:
			print "LoadingWindow.Open.LoadImage - %s File Load Error" % (imgFileName)
			self.loadingImage.Hide()


		width = float(wndMgr.GetScreenWidth()) / float(self.loadingImage.GetWidth())
		height = float(wndMgr.GetScreenHeight()) / float(self.loadingImage.GetHeight())

		self.loadingImage.SetScale(width, height)
		self.loadingGage.SetPercentage(2, 100)

		self.Show()

		chrSlot=self.stream.GetCharacterSlot()
		net.SendSelectCharacterPacket(chrSlot)

		app.SetFrameSkip(0)	

	def Close(self):
		app.SetFrameSkip(1)

		self.loadStepList = []
		
		if self.loadingImage:
			self.loadingImage.Hide()
			self.loadingImage = None
		
		if self.loadingGage:
			self.loadingGage.Hide()
			self.loadingGage = None
		
		if self.errMsg:
			self.errMsg.Hide()
			self.errMsg = None
			
		self.ClearDictionary()
		self.Hide()		

	def OnPressEscapeKey(self):
		app.SetFrameSkip(1)
		self.stream.SetLoginPhase()
		return True

	def __SetNext(self, next):
		if next:
			self.update=ui.__mem_func__(next)
		else:
			self.update=0

	def __SetProgress(self, p):
		if self.loadingGage:
			percentage = 2 + 98 * p / 100
			self.loadingGage.SetPercentage(percentage, 100)

			self.loadingInfo.SetText("£adowanie: {}%".format(p))  

	def LoadData(self, playerX, playerY):
		self.playerX = playerX
		self.playerY = playerY

		self.__RegisterDungeonMapName()
		self.__RegisterSkill()
		self.__RegisterTitleName()
		self.__RegisterColor()
		self.__RegisterEmotionIcon()

		self.loadStepList = [
			(0, ui.__mem_func__(self.__InitData)),
			(10, ui.__mem_func__(self.__LoadMap)),
			(30, ui.__mem_func__(self.__LoadSound)),
			(40, ui.__mem_func__(self.__LoadEffect)),
			(50, ui.__mem_func__(self.__LoadWarrior)),
			(60, ui.__mem_func__(self.__LoadAssassin)),
			(70, ui.__mem_func__(self.__LoadSura)),
			(80, ui.__mem_func__(self.__LoadShaman)),
			(90, ui.__mem_func__(self.__LoadSkill)),
			(93, ui.__mem_func__(self.__LoadEnemy)),
			(97, ui.__mem_func__(self.__LoadNPC)),
			(98, ui.__mem_func__(self.__LoadGuildBuilding)),
			(99, ui.__mem_func__(self.__LoadRaceHeight)),
			(100, ui.__mem_func__(self.__StartGame)),
		]
		
		if not getattr(self, 'isLoaded', False):
			min_step, max_step = self.loadStepList[0][0], self.loadStepList[-1][0]
			existing_steps = {step[0] for step in self.loadStepList}
			missing_steps = set(range(min_step, max_step + 1)) - existing_steps
        
			self.loadStepList += [(progress, self.__DummyLoadFunction) for progress in missing_steps]
			self.loadStepList.sort(key=lambda x: x[0])
        
			self.isLoaded = True
    
		self.__SetProgress(0)

	def OnUpdate(self):
		if self.loadStepList:
			progress, runFunc = self.loadStepList.pop(0)

			try:
				if runFunc is not None:
					runFunc()
			except:	
				self.errMsg.Show()
				self.loadStepList=[]
				app.Exit()
				return

			self.__SetProgress(progress)
			
	def __DummyLoadFunction(self):
		pass

	def __InitData(self):
		playerSettingModule.LoadGameData("INIT")

	def __RegisterDungeonMapName(self):
		background.RegisterDungeonMapName("map_monkey_dungeon_easy")

	def __RegisterSkill(self):
		race = net.GetMainActorRace()
		group = net.GetMainActorSkillGroup()
		empire = net.GetMainActorEmpire()

		playerSettingModule.RegisterSkill(race, group, empire)

	def __RegisterTitleName(self):

		def GetTitleData(gender):
			return localeInfo.TITLE_NAME_DICT.get(gender, ('UNKNOWN_GENDER', ))

		def GetTitleNamesTuple(*title_data):
			titleNamesTuple = map(iter, title_data)
			while titleNamesTuple:
				yield tuple(map(next, titleNamesTuple))

		for titleNameIndex, (titleNameFemale, titleNameMale) in enumerate(GetTitleNamesTuple(GetTitleData(localeInfo.GENDER_FEMALE), GetTitleData(localeInfo.GENDER_MALE))):
			chrmgr.RegisterTitleName(titleNameIndex, titleNameFemale, titleNameMale)

	def __RegisterColor(self):

		base_name_color_dict = {
			chrmgr.NAMECOLOR_PC:colorInfo.CHR_NAME_RGB_PC,
			chrmgr.NAMECOLOR_NPC:colorInfo.CHR_NAME_RGB_NPC,
			chrmgr.NAMECOLOR_MOB:colorInfo.CHR_NAME_RGB_MOB,
			chrmgr.NAMECOLOR_PVP:colorInfo.CHR_NAME_RGB_PVP,
			chrmgr.NAMECOLOR_PK:colorInfo.CHR_NAME_RGB_PK,
			chrmgr.NAMECOLOR_PARTY:colorInfo.CHR_NAME_RGB_PARTY,
			chrmgr.NAMECOLOR_WARP:colorInfo.CHR_NAME_RGB_WARP,
			chrmgr.NAMECOLOR_WAYPOINT:colorInfo.CHR_NAME_RGB_WAYPOINT,
			chrmgr.NAMECOLOR_EMPIRE_MOB:colorInfo.CHR_NAME_RGB_EMPIRE_MOB,
			chrmgr.NAMECOLOR_EMPIRE_NPC:colorInfo.CHR_NAME_RGB_EMPIRE_NPC,
		}
		
		special_name_color_dict = {
			chrmgr.NAMECOLOR_EMPIRE_PC + i: getattr(colorInfo, 'CHR_NAME_RGB_EMPIRE_PC_{}'.format(chr(65 + i)))
			for i in range(3)
		}		
		
		name_color_dict = dict(base_name_color_dict.items() + special_name_color_dict.items())
    
		for name, rgb in name_color_dict.items():
			chrmgr.RegisterNameColor(name, *rgb)

		title_color_tuple = (colorInfo.TITLE_RGB_GOOD_4, colorInfo.TITLE_RGB_GOOD_3, colorInfo.TITLE_RGB_GOOD_2, 
							colorInfo.TITLE_RGB_GOOD_1, colorInfo.TITLE_RGB_NORMAL, colorInfo.TITLE_RGB_EVIL_1,
							colorInfo.TITLE_RGB_EVIL_2, colorInfo.TITLE_RGB_EVIL_3, colorInfo.TITLE_RGB_EVIL_4)
								
		for count, rgb in enumerate(title_color_tuple):
			chrmgr.RegisterTitleColor(count, *rgb)

	def __RegisterEmotionIcon(self):
		emotion.RegisterEmotionIcons()

	def __LoadMap(self):
		net.Warp(self.playerX, self.playerY)

	def __LoadSound(self):
		playerSettingModule.LoadGameData("SOUND")

	def __LoadEffect(self):
		playerSettingModule.LoadGameData("EFFECT")

	def __LoadWarrior(self):
		playerSettingModule.LoadGameData("WARRIOR")

	def __LoadAssassin(self):
		playerSettingModule.LoadGameData("ASSASSIN")

	def __LoadSura(self):
		playerSettingModule.LoadGameData("SURA")

	def __LoadShaman(self):
		playerSettingModule.LoadGameData("SHAMAN")

	def __LoadSkill(self):
		playerSettingModule.LoadGameData("SKILL")

	def __LoadEnemy(self):
		playerSettingModule.LoadGameData("ENEMY")

	def __LoadNPC(self):
		playerSettingModule.LoadGameData("NPC")
		
	def __LoadRaceHeight(self):
		playerSettingModule.LoadGameData("RACE_HEIGHT")

	def __LoadGuildBuilding(self):
		playerSettingModule.LoadGuildBuildingList(localeInfo.GUILD_BUILDING_LIST_TXT)

	def __StartGame(self):
		background.SetViewDistanceSet(background.DISTANCE0, 25600)
		background.SelectViewDistanceNum(background.DISTANCE0)
		app.SetGlobalCenterPosition(self.playerX, self.playerY)
		net.StartGame()
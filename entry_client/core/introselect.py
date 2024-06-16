# --------------------------------------------------------------------------
# Title: Nowe kodowanie intro select like official (zoptymalizowana wersja)
# Author: Sevence
# Website: Sharegon.pl
# Date: 15.09.2023r.
# --------------------------------------------------------------------------
import chr
import grp
import app
import net
import snd
import ui
import event
import wndMgr
import player
import uiTarget
import uiCommon
import uiTaskBar
import uiToolTip
import musicInfo
import localeInfo
import uiCharacter
import uiInventory
import systemSetting
import uiPlayerGauge
import networkModule
import uiScriptLocale
import uiAffectShower
import uiMapNameShower
import interfaceModule
import playerSettingModule

#############################################################
M2_INIT_VALUE = -1
JOB_WARRIOR = 0
JOB_ASSASSIN = 1
JOB_SURA = 2
JOB_SHAMAN = 3
CHARACTER_SLOT_COUNT_MAX = 5
MAX_LENGTH_PRIVATE_CODE = 7

ENABLE_ENGNUM_DELETE_CODE = False

BASE_PATH = "D:/ymir work/"
BASE_ICON_PATH = BASE_PATH + "bin/icon/face/"
BASE_FACE_PATH = BASE_PATH + "ui/intro/public_intro/face/"
#############################################################

class MyCharacters:
	class MyUnit:
		def __init__(self, const_id, name, level, race, playtime, guildname, form, hair, stat_str, stat_dex, stat_hth, stat_int, change_name, last_playtime):
			self.UnitDataDic = {
				"ID": const_id,
				"NAME": name,
				"LEVEL": level,
				"RACE": race,
				"PLAYTIME": playtime,
				"GUILDNAME": guildname,
				"FORM": form,
				"HAIR": hair,
				"STR": stat_str,
				"DEX": stat_dex,
				"HTH": stat_hth,
				"INT": stat_int,
				"CHANGENAME":	change_name,
				"LASTPLAYTIME": last_playtime,
			}

		def __del__(self):
			self.UnitDataDic = None
		
		def GetUnitData(self):
			return self.UnitDataDic

	def __init__(self, stream):
		self.MainStream = stream
		self.PriorityData = []
		self.myUnitDic = {}
		self.HowManyChar = 0
		self.EmptySlot = []

		self.Race = [None] * 5
		self.Job = [None] * 5
		self.Guild_Name = [None] * 5
		self.Play_Time = [None] * 5
		self.Change_Name = [None] * 5
		self.Stat_Point = dict.fromkeys(xrange(5), None)

	def Clear(self):
		self.MainStream = None 
		
		for i in xrange(self.HowManyChar):
			chr.DeleteInstance(i)
			
		self.PriorityData = None
		self.myUnitDic = None
		self.HowManyChar = 0
		self.EmptySlot	= None
		self.Race = None
		self.Job = None 		
		self.Guild_Name = None
		self.Play_Time = None
		self.Change_Name = None
		self.Stat_Point = None

	def __del__(self):
		self.Clear()
			
	def GetSlotDataInt(self, slot_index, data_type):
		return net.GetAccountCharacterSlotDataInteger(slot_index, data_type)
		
	def GetSlotDataStr(self, slot_index, data_type):
		return net.GetAccountCharacterSlotDataString(slot_index, data_type)			
			
	def LoadCharacterData(self):
		self.RefreshData()
		self.MainStream.All_ButtonInfoHide()
		
		for i in xrange(CHARACTER_SLOT_COUNT_MAX):
			pid = self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_ID)
			
			if not pid:
				self.EmptySlot.append(i)
				continue

			char_data = {
				"name": self.GetSlotDataStr(i, net.ACCOUNT_CHARACTER_SLOT_NAME),
				"level": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_LEVEL),
				"race": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_RACE),
				"playtime": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_PLAYTIME),
				"guildname": self.GetSlotDataStr(i, net.ACCOUNT_CHARACTER_SLOT_GUILD_NAME),
				"form": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_FORM),
				"hair": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_HAIR),
				"stat_str": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_STR),
				"stat_dex": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_DEX),
				"stat_hth": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_HTH),
				"stat_int": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_INT),
				"last_playtime": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_LAST_PLAYTIME),
				"change_name": self.GetSlotDataInt(i, net.ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG)
			}

			self.SetPriorityData(char_data["last_playtime"], i)
			self.myUnitDic[i] = self.MyUnit(i, **char_data)

		self.PriorityData.sort(reverse = True)

		for i in xrange(len(self.PriorityData)):
			time, index = self.PriorityData[i]
			DestDataDic = self.myUnitDic[index].GetUnitData()

			self.SetSortingData(i, DestDataDic["RACE"], DestDataDic["GUILDNAME"], DestDataDic["PLAYTIME"], DestDataDic["STR"], DestDataDic["DEX"], DestDataDic["HTH"], DestDataDic["INT"], DestDataDic["CHANGENAME"])
			self.MakeCharacter(i, DestDataDic["NAME"], DestDataDic["RACE"], DestDataDic["FORM"], DestDataDic["HAIR"])
			self.MainStream.InitDataSet(i, DestDataDic["NAME"], DestDataDic["LEVEL"], DestDataDic["ID"])

		if self.HowManyChar:
			self.MainStream.SelectButton(0)

		return self.HowManyChar
		
	def SetPriorityData(self, last_playtime, index):
		self.PriorityData.append([last_playtime, index])

	def MakeCharacter(self, slot, name, race, form, hair):
		chr.CreateInstance(slot)
		chr.SelectInstance(slot)
		chr.SetVirtualID(slot)
		chr.SetNameString(name)

		chr.SetRace(race)
		chr.SetArmor(form)
		chr.SetHair(hair)

		chr.SetMotionMode(chr.MOTION_MODE_GENERAL)
		chr.SetLoopMotion(chr.MOTION_INTRO_WAIT)

		chr.SetRotation(0.0)
		chr.Hide()

	def SetSortingData(self, slot, race, guildname, playtime, pStr, pDex, pHth, pInt, changename):	
		self.HowManyChar += 1
		self.Race[slot] = race
		self.Job[slot] = chr.RaceToJob(race)
		self.Guild_Name[slot] = guildname
		self.Play_Time[slot] = playtime
		self.Change_Name[slot] = changename
		self.Stat_Point[slot] = [pHth, pInt, pStr, pDex]

	def GetRace(self, slot):
		return self.Race[slot]

	def GetJob(self, slot):
		return self.Job[slot]

	def GetMyCharacterCount(self):
		return self.HowManyChar

	def GetEmptySlot(self):
		if not len(self.EmptySlot):
			return M2_INIT_VALUE

		return self.EmptySlot[0]

	def GetStatPoint(self, slot):
		return self.Stat_Point[slot]

	def GetGuildNamePlayTime(self, slot):
		return self.Guild_Name[slot], self.Play_Time[slot]

	def GetChangeName(self, slot):
		return self.Change_Name[slot]

	def SetChangeNameSuccess(self, slot):
		self.Change_Name[slot] = 0

	def RefreshData(self):
		self.HowManyChar = 0
		self.EmptySlot = []
		self.PriorityData = []
		self.Race = [None] * 5
		self.Guild_Name = [None] * 5
		self.Play_Time = [None] * 5
		self.Change_Name = [None] * 5
		self.Stat_Point = dict.fromkeys(xrange(5), None)

class SelectCharacterWindow(ui.Window):
	EMPIRE_NAME = { 
		net.EMPIRE_A : localeInfo.EMPIRE_A, 
		net.EMPIRE_B : localeInfo.EMPIRE_B, 
		net.EMPIRE_C : localeInfo.EMPIRE_C 
	}

	EMPIRE_NAME_COLOR = {
		net.EMPIRE_A : (0.7450, 0, 0),
		net.EMPIRE_B : (0.8666, 0.6156, 0.1843),
		net.EMPIRE_C : (0.2235, 0.2549, 0.7490) 
	}

	RACE_FACE_PATH = {
		playerSettingModule.RACE_WARRIOR_M: BASE_FACE_PATH + "face_warrior_m_0",
		playerSettingModule.RACE_ASSASSIN_W: BASE_FACE_PATH + "face_assassin_w_0",
		playerSettingModule.RACE_SURA_M: BASE_FACE_PATH + "face_sura_m_0",
		playerSettingModule.RACE_SHAMAN_W: BASE_FACE_PATH + "face_shaman_w_0",
		playerSettingModule.RACE_WARRIOR_W: BASE_FACE_PATH + "face_warrior_w_0",
		playerSettingModule.RACE_ASSASSIN_M: BASE_FACE_PATH + "face_assassin_m_0",
		playerSettingModule.RACE_SURA_W: BASE_FACE_PATH + "face_sura_w_0",
		playerSettingModule.RACE_SHAMAN_M: BASE_FACE_PATH + "face_shaman_m_0",
	}

	DISC_FACE_PATH = {
		playerSettingModule.RACE_WARRIOR_M: BASE_ICON_PATH + "warrior_m.tga",
		playerSettingModule.RACE_ASSASSIN_W: BASE_ICON_PATH + "assassin_w.tga",
		playerSettingModule.RACE_SURA_M: BASE_ICON_PATH + "sura_m.tga",
		playerSettingModule.RACE_SHAMAN_W: BASE_ICON_PATH + "shaman_w.tga",
		playerSettingModule.RACE_WARRIOR_W: BASE_ICON_PATH + "warrior_w.tga",
		playerSettingModule.RACE_ASSASSIN_M: BASE_ICON_PATH + "assassin_m.tga",
		playerSettingModule.RACE_SURA_W: BASE_ICON_PATH + "sura_w.tga",
		playerSettingModule.RACE_SHAMAN_M: BASE_ICON_PATH + "shaman_m.tga",
	}

	DESCRIPTION_FILE_NAME = (
		uiScriptLocale.JOBDESC_WARRIOR_PATH,
		uiScriptLocale.JOBDESC_ASSASSIN_PATH,
		uiScriptLocale.JOBDESC_SURA_PATH,
		uiScriptLocale.JOBDESC_SHAMAN_PATH,
	)

	JOB_LIST = {0 : localeInfo.JOB_WARRIOR, 1 : localeInfo.JOB_ASSASSIN, 2 : localeInfo.JOB_SURA, 3 : localeInfo.JOB_SHAMAN}
	
	BUTTON_KEY_MAPPING = {2: 0, 3: 1, 4: 2, 5: 3, 6: 4}	

	class DescriptionBox(ui.Window):
		def __init__(self):
			ui.Window.__init__(self)
			self.descIndex = 0

		def __del__(self):
			ui.Window.__del__(self)

		def SetIndex(self, index):
			self.descIndex = index

		def OnRender(self):
			event.RenderEventSet(self.descIndex)

	class CharacterRenderer(ui.Window):
		def OnRender(self):
			grp.ClearDepthBuffer()
			self.SetupRenderState()
			
			screenWidth = wndMgr.GetScreenWidth()
			screenHeight = wndMgr.GetScreenHeight()
			newScreenWidth = float(screenWidth)
			newScreenHeight = float(screenHeight)

			grp.SetViewport(0.0, 0.0, newScreenWidth/screenWidth, newScreenHeight/screenHeight)

			app.SetCenterPosition(0.0, 0.0, 0.0)
			app.SetCamera(1550.0, 15.0, 180.0, 95.0)
			grp.SetPerspective(10.0, newScreenWidth/newScreenHeight, 1000.0, 3000.0)
        
			x, y = app.GetCursorPosition()
			grp.SetCursorPosition(x, y)

			chr.Deform()
			chr.Render()
					
			self.ResetRenderState()
			
		def SetupRenderState(self):
			grp.SetGameRenderState()
			grp.PushState()
			grp.SetOmniLight()
		   
		def ResetRenderState(self):
			grp.RestoreViewport()
			grp.PopState()
			grp.SetInterfaceRenderState()
	
	def __init__(self, stream):
		ui.Window.__init__(self)
		self.InitializeVariables(stream)

	def InitializeVariables(self, stream):
		net.SetPhaseWindow(net.PHASE_WINDOW_SELECT, self)
		self.stream = stream

		self.SelectSlot = M2_INIT_VALUE
		self.SelectEmpire = False
		self.ShowToolTip = False
		self.select_job = M2_INIT_VALUE
		self.select_race = M2_INIT_VALUE
		self.LEN_STATPOINT = 4
		self.descIndex = 0
		self.statpoint = [0] * self.LEN_STATPOINT
		self.curGauge = [0.0] * self.LEN_STATPOINT

		self.DefineFontColors()

		self.Not_SelectMotion = False
		self.MotionStart = False
		self.MotionTime = 0.0
		self.RealSlot = []
		self.Disable = False
	
	def DefineFontColors(self):
		self.Name_FontColor_Def = grp.GenerateColor(0.7215, 0.7215, 0.7215, 1.0)
		self.Name_FontColor = grp.GenerateColor(197.0/255.0, 134.0/255.0, 101.0/255.0, 1.0)
		self.Level_FontColor = grp.GenerateColor(250.0/255.0, 211.0/255.0, 136.0/255.0, 1.0)

	def __del__(self):
		ui.Window.__del__(self)
		net.SetPhaseWindow(net.PHASE_WINDOW_SELECT, 0)

	def Open(self):
		playerSettingModule.LoadGameData("INIT")
		
		self.dlgBoard = ui.ScriptWindow()
		pythonScriptLoader = ui.PythonScriptLoader()
		pythonScriptLoader.LoadScriptFile(self.dlgBoard, uiScriptLocale.LOCALE_UISCRIPT_PATH + "selectcharacterwindow.py")		

		getChild = self.dlgBoard.GetChild
		
		self.backGroundDict = {
			net.EMPIRE_B: "d:/ymir work/ui/intro/empire/background/empire_chunjo.sub",
			net.EMPIRE_C: "d:/ymir work/ui/intro/empire/background/empire_jinno.sub",
		}		
		
		self.backGround = getChild("BackGround")

		self.NameList = [getChild("name_{}".format(name)) for name in ["warrior", "assassin", "sura", "shaman"]]

		self.empireName = getChild("EmpireName")
		self.flagDict = {
			net.EMPIRE_B: "d:/ymir work/ui/intro/empire/empireflag_b.sub",
			net.EMPIRE_C: "d:/ymir work/ui/intro/empire/empireflag_c.sub",
		}
		self.flag = getChild("EmpireFlag")

		self.btnStart = getChild("start_button")
		self.btnCreate = getChild("create_button")
		self.btnDelete = getChild("delete_button")
		self.btnExit = getChild("exit_button")
		
		self.FaceImage = [getChild("CharacterFace_{}".format(i)) for i in xrange(5)]
		self.CharacterButtonList = [getChild("CharacterSlot_{}".format(i)) for i in xrange(5)]
		
		for idx, button in enumerate(self.CharacterButtonList):
			button.ShowToolTip = lambda arg = idx: self.OverInToolTip(arg)
			button.HideToolTip = lambda: self.OverOutToolTip()
		
		btn_names_with_locale = [("create_button", uiScriptLocale.SELECT_CREATE), ("delete_button", uiScriptLocale.SELECT_DELETE),
							("start_button", uiScriptLocale.SELECT_SELECT), ("exit_button", uiScriptLocale.SELECT_EXIT),
							("prev_button", uiScriptLocale.CREATE_PREV), ("next_button", uiScriptLocale.CREATE_NEXT)]

		for btn_name, locale_value in btn_names_with_locale:
			btn = getChild(btn_name)
			btn.ShowToolTip = lambda arg = locale_value: self.OverInToolTipETC(arg)
			btn.HideToolTip = lambda: self.OverOutToolTip()

		self.statValue = [getChild("{}_value".format(name)) for name in ["hth", "int", "str", "dex"]]
		self.GaugeList = [getChild("{}_gauge".format(name)) for name in ["hth", "int", "str", "dex"]]

		self.textBoard = getChild("text_board")
		self.btnPrev = getChild("prev_button")
		self.btnNext = getChild("next_button")

		self.discFace = getChild("DiscFace")
		self.raceNameText = getChild("raceName_Text")
		self.myID = getChild("my_id")

		self.btnStart.SetEvent(ui.__mem_func__(self.StartGameButton))
		self.btnCreate.SetEvent(ui.__mem_func__(self.CreateCharacterButton))
		self.btnExit.SetEvent(ui.__mem_func__(self.ExitButton))
		self.btnDelete.SetEvent(ui.__mem_func__(self.InputPrivateCode))	

		for idx, button in enumerate(self.CharacterButtonList):
			button.SetEvent(ui.__mem_func__(self.SelectButton), idx)

		for idx, image in enumerate(self.FaceImage):
			for event_name in ["mouse_click", "mouse_over_in", "mouse_over_out"]:
				image.SetEvent(ui.__mem_func__(self.EventProgress), event_name, idx)

		self.btnPrev.SetEvent(ui.__mem_func__(self.PrevDescriptionPage))
		self.btnNext.SetEvent(ui.__mem_func__(self.NextDescriptionPage))

		self.mycharacters = MyCharacters(self);
		self.mycharacters.LoadCharacterData()

		if not self.mycharacters.GetMyCharacterCount():
			self.stream.SetCharacterSlot(self.mycharacters.GetEmptySlot())
			self.SelectEmpire = True

		self.descriptionBox = self.DescriptionBox()
		self.descriptionBox.Show()

		self.toolTip = uiToolTip.ToolTip()
		self.toolTip.ClearToolTip()

		self.dlgBoard.Show()
		self.Show()

		my_empire = net.GetEmpireID()
		self.SetEmpire(my_empire)

		app.ShowCursor()

		if musicInfo.selectMusic != "":
			snd.SetMusicVolume(systemSetting.GetMusicVolume())
			snd.FadeInMusic("BGM/"+musicInfo.selectMusic)

		self.chrRenderer = self.CharacterRenderer()
		self.chrRenderer.SetParent(self.backGround)
		self.chrRenderer.Show()

	def EventProgress(self, event_type, slot):
		if self.Disable:
			return

		if "mouse_click" == event_type:
			if slot != self.SelectSlot:
				snd.PlaySound("sound/ui/click.wav")
				self.SelectButton(slot)

		elif "mouse_over_in" == event_type:
			self.ResetButtonsState()
			self.CharacterButtonList[self.SelectSlot].Down()
			self.OverInToolTip(slot)

		elif "mouse_over_out" == event_type:
			self.ResetButtonsState()
			self.CharacterButtonList[self.SelectSlot].Down()
			self.OverOutToolTip()

		else:
			print " New_introSelect.py ::EventProgress : FALSE"
		
	def ResetButtonsState(self):
		for button in self.CharacterButtonList:
			button.SetUp()		
		
	def SelectButton(self, slot):		
		if slot >= self.mycharacters.GetMyCharacterCount() or slot == self.SelectSlot:
			return
			
		self.ResetButtonsState()
			
		if self.Not_SelectMotion or self.MotionTime != 0.0:
			self.CharacterButtonList[slot].Over()
			return

		self.SelectSlot = slot
		self.CharacterButtonList[self.SelectSlot].Down()
		self.stream.SetCharacterSlot(self.RealSlot[self.SelectSlot])

		self.select_job = self.mycharacters.GetJob(self.SelectSlot)
		
		event.ClearEventSet(self.descIndex)
		self.descIndex = event.RegisterEventSet(self.DESCRIPTION_FILE_NAME[self.select_job])
		event.SetFontColor(self.descIndex, 0.7843, 0.7843, 0.7843)
		event.SetRestrictedCount(self.descIndex, 35)

		if event.BOX_VISIBLE_LINE_COUNT >= event.GetTotalLineCount(self.descIndex):
			self.btnPrev.Hide()
			self.btnNext.Hide()
		else:
			self.btnPrev.Show()
			self.btnNext.Show()

		self.ResetStat()
		
		for i in self.NameList:
			i.SetAlpha(0)
		self.NameList[self.select_job].SetAlpha(1)
		
		self.select_race = self.mycharacters.GetRace(self.SelectSlot)
		for i in xrange(self.mycharacters.GetMyCharacterCount()):
			if slot == i:
				self.FaceImage[slot].LoadImage(self.RACE_FACE_PATH[self.select_race] + "1.sub")
				self.CharacterButtonList[slot].SetAppendTextColor(0, self.Name_FontColor)
			else:
				self.FaceImage[i].LoadImage(self.RACE_FACE_PATH[self.mycharacters.GetRace(i)] + "2.sub")
				self.CharacterButtonList[i].SetAppendTextColor(0, self.Name_FontColor_Def)

			self.discFace.LoadImage(self.DISC_FACE_PATH[self.select_race])
		self.raceNameText.SetText(self.JOB_LIST[self.select_job])

		chr.Hide()
		chr.SelectInstance(self.SelectSlot)
		chr.Show()

	def Close(self):
		del self.mycharacters
		
		# Paths & Descriptions
		self.EMPIRE_NAME = None
		self.EMPIRE_NAME_COLOR = None
		self.RACE_FACE_PATH = None
		self.DISC_FACE_PATH = None
		self.DESCRIPTION_FILE_NAME = None
		self.JOB_LIST = None
		
		# Default Values
		self.SelectSlot = None
		self.SelectEmpire = None
		self.ShowToolTip = None
		self.LEN_STATPOINT = None
		self.descIndex = None
		self.statpoint = None
		self.curGauge = None
		self.Name_FontColor_Def = None
		self.Name_FontColor = None
		self.Level_FontColor = None
		self.Not_SelectMotion = None
		self.MotionStart = None
		self.MotionTime = None
		self.RealSlot = None
	
		# Selected Attributes
		self.select_job = None
		self.select_race = None
		
		# UI Components
		self.dlgBoard = None
		self.backGround = None
		self.backGroundDict = None
		self.NameList = None
		self.empireName = None
		self.flag = None
		self.flagDict = None
		self.btnStart = None
		self.btnCreate = None
		self.btnDelete = None
		self.btnExit = None
		self.FaceImage = None
		self.CharacterButtonList = None
		self.statValue = None
		self.GaugeList = None
		self.textBoard = None
		self.btnPrev = None
		self.btnNext = None
		self.raceNameText = None
		self.myID = None

		# Additional Components
		self.descriptionBox = None
		self.toolTip = None
		self.Disable = None

		if musicInfo.selectMusic != "":
			snd.FadeOutMusic("BGM/" + musicInfo.selectMusic)

		self.Hide()
		self.KillFocus()
		app.HideCursor()
		event.Destroy()
		
	def SetEmpire(self, empire_id):
		self.empireName.SetText(self.EMPIRE_NAME.get(empire_id, ""))

		if empire_id in self.EMPIRE_NAME_COLOR:
			rgb = self.EMPIRE_NAME_COLOR[empire_id]
			self.empireName.SetFontColor(rgb[0], rgb[1], rgb[2])
		else:
			pass

		if empire_id != net.EMPIRE_A:
			self.flag.LoadImage(self.flagDict[empire_id])
			self.flag.SetScale(0.45, 0.45)
			self.backGround.LoadImage(self.backGroundDict[empire_id])
			self.backGround.SetScale(float(wndMgr.GetScreenWidth()) / 1024.0, float(wndMgr.GetScreenHeight()) / 768.0)
						
	def CreateCharacterButton(self):
		slotNumber = self.mycharacters.GetEmptySlot()
			
		if slotNumber == M2_INIT_VALUE :
			self.stream.popupWindow.Close()
			self.stream.popupWindow.Open(localeInfo.CREATE_FULL, 0, localeInfo.UI_OK)
			return
			
		stream = self.stream
	
		if not self.GetCharacterSlotPID(slotNumber):
			stream.SetCharacterSlot(slotNumber)

			if not self.mycharacters.GetMyCharacterCount():
				self.SelectEmpire = True
			else:
				stream.SetCreateCharacterPhase()
				self.Hide()
		
	def ExitButton(self):
		self.stream.SetLoginPhase()		
		self.Hide()
		
	def StartGameButton(self):
		if not self.mycharacters.GetMyCharacterCount() or self.MotionTime != 0.0:
			return
		
		self.DisableWindow()
		
		if self.mycharacters.GetChangeName(self.SelectSlot):
			self.OpenChangeNameDialog()
			return

		chr.PushOnceMotion(chr.MOTION_INTRO_SELECTED)
		self.MotionStart = True
		self.MotionTime = app.GetTime()
				
	def OnUpdate(self):
		chr.Update()
		self.ToolTipProgress()

		if self.SelectEmpire:
			self.SelectEmpire = False
			self.stream.SetReselectEmpirePhase()
			self.Hide()

		currentTime = app.GetTime()
		if self.MotionStart and currentTime - self.MotionTime >= 2.0:
			self.MotionStart = False
			chrSlot = self.stream.GetCharacterSlot()

			if musicInfo.selectMusic:
				snd.FadeLimitOutMusic("BGM/" + musicInfo.selectMusic, systemSetting.GetMusicVolume() * 0.05)

			net.DirectEnter(chrSlot)
			playTime = net.GetAccountCharacterSlotDataInteger(chrSlot, net.ACCOUNT_CHARACTER_SLOT_PLAYTIME)

			import player
			player.SetPlayTime(playTime)
			import chat
			chat.Clear()

		xposEventSet, yposEventSet = self.textBoard.GetGlobalPosition()
		event.UpdateEventSet(self.descIndex, xposEventSet + 7, -(yposEventSet + 7))
		self.descriptionBox.SetIndex(self.descIndex)

		for gauge, cur in zip(self.GaugeList, self.curGauge):
			gauge.SetPercentage(cur, 1.0)
	
	def GetCharacterSlotPID(self, slotIndex):
		return net.GetAccountCharacterSlotDataInteger(slotIndex, net.ACCOUNT_CHARACTER_SLOT_ID)
	
	def All_ButtonInfoHide(self) :
		for i in xrange(CHARACTER_SLOT_COUNT_MAX):
			self.CharacterButtonList[i].Hide()
			self.FaceImage[i].Hide()
			
	def InitDataSet(self, slot, name, level, real_slot):
		char_button = self.CharacterButtonList[slot]
		width = char_button.GetWidth()
		height = char_button.GetHeight()

		face_image_path = self.RACE_FACE_PATH[self.mycharacters.GetRace(slot)] + "2.sub"

		char_button.AppendTextLine(name, localeInfo.UI_DEF_FONT, self.Name_FontColor_Def, "right", width - 12, height/4 + 2)
		char_button.AppendTextLine("Lv." + str(level), localeInfo.UI_DEF_FONT, self.Level_FontColor, "left", width - 42, height*3/4)
		char_button.Show()

		self.FaceImage[slot].LoadImage(face_image_path)
		self.FaceImage[slot].Show()

		self.RealSlot.append(real_slot)
				
	def InputPrivateCode(self):
		if not self.mycharacters.GetMyCharacterCount():
			return
			
		privateInputBoard = uiCommon.InputDialogWithDescription()
		privateInputBoard.SetTitle(localeInfo.INPUT_PRIVATE_CODE_DIALOG_TITLE)
		privateInputBoard.SetAcceptEvent(ui.__mem_func__(self.AcceptInputPrivateCode))
		privateInputBoard.SetCancelEvent(ui.__mem_func__(self.CancelInputPrivateCode))
		
		if not ENABLE_ENGNUM_DELETE_CODE:
			privateInputBoard.SetNumberMode()
			
		privateInputBoard.SetSecretMode()
		privateInputBoard.SetMaxLength(MAX_LENGTH_PRIVATE_CODE)
			
		privateInputBoard.SetBoardWidth(250)
		privateInputBoard.SetDescription(localeInfo.INPUT_PRIVATE_CODE_DIALOG_DESCRIPTION)
		privateInputBoard.Open()
		self.privateInputBoard = privateInputBoard

		self.DisableWindow()

		if not self.Not_SelectMotion:
			self.Not_SelectMotion = True
			chr.PushOnceMotion(chr.MOTION_INTRO_NOT_SELECTED, 0.1)
		
	def AcceptInputPrivateCode(self):
		privateCode = self.privateInputBoard.GetText()
		
		if not privateCode:
			return
		
		pid = net.GetAccountCharacterSlotDataInteger(self.RealSlot[self.SelectSlot], net.ACCOUNT_CHARACTER_SLOT_ID)
		if not pid:
			self.PopupMessage(localeInfo.SELECT_EMPTY_SLOT)
			return

		net.SendDestroyCharacterPacket(self.RealSlot[self.SelectSlot], privateCode)
		self.PopupMessage(localeInfo.SELECT_DELEING)
		self.CancelInputPrivateCode()
		
		return True
	
	def CancelInputPrivateCode(self) :
		self.privateInputBoard = None
		self.Not_SelectMotion = False
		chr.SetLoopMotion(chr.MOTION_INTRO_WAIT)
		self.EnableWindow()
		return True

	def OnDeleteSuccess(self, slot):
		self.PopupMessage(localeInfo.SELECT_DELETED)
		for i in xrange(len(self.RealSlot)):
			chr.DeleteInstance(i)
			
		self.RealSlot = []
		self.SelectSlot = M2_INIT_VALUE
		
		for button in self.CharacterButtonList:
			button.AppendTextLineAllClear()

		if not self.mycharacters.LoadCharacterData():
			self.stream.popupWindow.Close()
			self.stream.SetCharacterSlot(self.mycharacters.GetEmptySlot())
			self.SelectEmpire = True
	
	def OnDeleteFailure(self):
		self.PopupMessage(localeInfo.SELECT_CAN_NOT_DELETE)
			
	def EmptyFunc(self):
		pass

	def PopupMessage(self, msg, func = 0):
		if func is 0:
			func = self.EmptyFunc

		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(msg, func, localeInfo.UI_OK)
	
	def RefreshStat(self):
		statSummary = 90.0 
		self.curGauge = [float(point) / statSummary for point in self.statpoint]

		for i in xrange(self.LEN_STATPOINT):
			self.statValue[i].SetText(str(self.statpoint[i]))

	def ResetStat(self):
		myStatPoint = self.mycharacters.GetStatPoint(self.SelectSlot)
		
		if not myStatPoint:
			return
		
		for i in xrange(self.LEN_STATPOINT):
			self.statpoint[i] = myStatPoint[i]
		
		self.RefreshStat()

	def PrevDescriptionPage(self):
		if event.IsWait(self.descIndex):
			if event.GetVisibleStartLine(self.descIndex) - event.BOX_VISIBLE_LINE_COUNT >= 0:
				event.SetVisibleStartLine(self.descIndex, event.GetVisibleStartLine(self.descIndex) - event.BOX_VISIBLE_LINE_COUNT)
		event.Skip(self.descIndex)

	def NextDescriptionPage(self):
		if event.IsWait(self.descIndex):
			event.SetVisibleStartLine(self.descIndex, event.GetVisibleStartLine(self.descIndex) + event.BOX_VISIBLE_LINE_COUNT)
		event.Skip(self.descIndex)

	def OverInToolTip(self, slot) :
		GuildName = localeInfo.GUILD_NAME
		myGuildName, myPlayTime = self.mycharacters.GetGuildNamePlayTime(slot)
		pos_x, pos_y = self.CharacterButtonList[slot].GetGlobalPosition()
		
		myGuildName = myGuildName or localeInfo.SELECT_NOT_JOIN_GUILD

		guild_name = "{} : {}".format(GuildName, myGuildName)
		play_time = "{} :".format(uiScriptLocale.SELECT_PLAYTIME)
		
		day, rem = divmod(myPlayTime, 60 * 24)
		hour, min = divmod(rem, 60)
		
		if day: 
			play_time += " {}{}".format(day, localeInfo.DAY)
		if hour:
			play_time += " {}{}".format(hour, localeInfo.HOUR)
		play_time += " {}{}".format(min, localeInfo.MINUTE)
	
		textlen = max(len(guild_name), len(play_time))
		tooltip_width = 6 * textlen + 22
		self.toolTip.ClearToolTip()
		self.toolTip.SetThinBoardSize(tooltip_width)
		self.toolTip.SetToolTipPosition(pos_x + 173 + tooltip_width / 2, pos_y + 34)
		self.toolTip.AppendTextLine(guild_name, 0xffe4cb1b, False)
		self.toolTip.AppendTextLine(play_time, 0xffffff00, False)
		self.toolTip.Show()
	
	def OverInToolTipETC(self, arg):
		arglen = len(str(arg))
		pos_x, pos_y = wndMgr.GetMousePosition()

		self.toolTip.ClearToolTip()
		self.toolTip.SetThinBoardSize(11 * arglen)
		self.toolTip.SetToolTipPosition(pos_x + 50, pos_y + 50)
		self.toolTip.AppendTextLine(arg, 0xffffff00)
		self.toolTip.Show()
		self.ShowToolTip = True
		
	def OverOutToolTip(self):
		self.toolTip.Hide()
		self.ShowToolTip = False
	
	def ToolTipProgress(self):
		if self.ShowToolTip:
			pos_x, pos_y = wndMgr.GetMousePosition()
			self.toolTip.SetToolTipPosition(pos_x + 50, pos_y + 50)
	
	def SameLoginDisconnect(self):
		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(localeInfo.LOGIN_FAILURE_SAMELOGIN, self.ExitButton, localeInfo.UI_OK)
		
	def OnIMEReturn(self):
		self.StartGameButton()
		return True
	
	def OnPressEscapeKey(self):
		self.ExitButton()
		return True
	
	def OnKeyDown(self, key):
		if self.MotionTime != 0.0:
			return

		if key in self.BUTTON_KEY_MAPPING:
			self.SelectButton(self.BUTTON_KEY_MAPPING[key])
		elif key in [200, 208]:
			self.KeyInputUpDown(key)
		else:
			return True

		return True

	def KeyInputUpDown(self, key):
		idx = self.SelectSlot
		maxValue = self.mycharacters.GetMyCharacterCount()

		if key == 200:
			idx = (idx - 1) % maxValue
		elif key == 208:
			idx = (idx + 1) % maxValue
		else:
			self.SelectButton(0)

		self.SelectButton(idx)

	def OnPressExitKey(self):
		self.ExitButton()
		return True
	
	def DisableWindow(self):
		self.btnStart.Disable()
		self.btnCreate.Disable()
		self.btnExit.Disable()
		self.btnDelete.Disable()
		self.btnPrev.Disable()
		self.btnNext.Disable()
		self.toolTip.Hide()
		self.ShowToolTip = False
		self.Disable = True
		for button in self.CharacterButtonList:
			button.Disable()

	def EnableWindow(self):
		self.btnStart.Enable()
		self.btnCreate.Enable()
		self.btnExit.Enable()
		self.btnDelete.Enable()
		self.btnPrev.Enable()
		self.btnNext.Enable()
		self.Disable = False
		for button in self.CharacterButtonList:
			button.Enable()

	def OpenChangeNameDialog(self):
		nameInputBoard = uiCommon.InputDialogWithDescription()
		nameInputBoard.SetTitle(localeInfo.SELECT_CHANGE_NAME_TITLE)
		nameInputBoard.SetAcceptEvent(ui.__mem_func__(self.AcceptInputName))
		nameInputBoard.SetCancelEvent(ui.__mem_func__(self.CancelInputName))
		nameInputBoard.SetMaxLength(chr.PLAYER_NAME_MAX_LEN)
		nameInputBoard.SetBoardWidth(200)
		nameInputBoard.SetDescription(localeInfo.SELECT_INPUT_CHANGING_NAME)
		nameInputBoard.Open()
		nameInputBoard.slot = self.RealSlot[self.SelectSlot]
		self.nameInputBoard = nameInputBoard
		
	def AcceptInputName(self):
		changeName = self.nameInputBoard.GetText()
		if not changeName:
			return

		net.SendChangeNamePacket(self.nameInputBoard.slot, changeName)
		return self.CancelInputName()

	def CancelInputName(self):
		self.nameInputBoard.Close()
		self.nameInputBoard = None
		self.EnableWindow()
		return True

	def OnCreateFailure(self, type):
		error_messages = {
			0: localeInfo.SELECT_CHANGE_FAILURE_STRANGE_NAME,
			1: localeInfo.SELECT_CHANGE_FAILURE_ALREADY_EXIST_NAME,
			100: localeInfo.SELECT_CHANGE_FAILURE_STRANGE_INDEX,
		}
		self.PopupMessage(error_messages.get(type, "Unknown Error"))
			
	def OnChangeName(self, slot, name):
		for i in xrange(len(self.RealSlot)):
			if self.RealSlot[i] == slot:
				self.ChangeNameButton(i, name)
				self.SelectButton(i)
				self.PopupMessage(localeInfo.SELECT_CHANGED_NAME)
				break

	def ChangeNameButton(self, slot, name):
		self.CharacterButtonList[slot].SetAppendTextChangeText(0, name)
		self.mycharacters.SetChangeNameSuccess(slot)
# --------------------------------------------------------------------------
# Title: Nowe kodowanie intro create like official (zoptymalizowana wersja)
# Author: Sevence
# Website: Sharegon.pl
# Date: 17.09.2023r.
# --------------------------------------------------------------------------

import ui
import chr
import grp
import app
import net
import snd
import event
import wndMgr
import uiToolTip
import musicInfo
import localeInfo
import networkModule
import systemSetting
import uiScriptLocale
import playerSettingModule

MAN = 0
WOMAN = 1

class CreateCharacterWindow(ui.Window):
	EMPIRE_NAME = {net.EMPIRE_A : localeInfo.EMPIRE_A, net.EMPIRE_B : localeInfo.EMPIRE_B, net.EMPIRE_C : localeInfo.EMPIRE_C,}
	EMPIRE_NAME_COLOR = {net.EMPIRE_A : (0.7450, 0, 0), net.EMPIRE_B : (0.8666, 0.6156, 0.1843), net.EMPIRE_C : (0.2235, 0.2549, 0.7490),}
	M2STATPOINT = ((4, 3, 6, 3 ),( 3, 3, 4, 6 ), ( 3, 5, 5, 3 ),( 4, 6, 3, 3 ),)
	
	M2JOBLIST = {
		0	:	localeInfo.JOB_WARRIOR,
		1	:	localeInfo.JOB_ASSASSIN,
		2	:	localeInfo.JOB_SURA,
		3	:	localeInfo.JOB_SHAMAN,
	}

	M2_CONST_ID = (
		(playerSettingModule.RACE_WARRIOR_M, playerSettingModule.RACE_WARRIOR_W),
		(playerSettingModule.RACE_ASSASSIN_M, playerSettingModule.RACE_ASSASSIN_W),
		(playerSettingModule.RACE_SURA_M, playerSettingModule.RACE_SURA_W),
		(playerSettingModule.RACE_SHAMAN_M, playerSettingModule.RACE_SHAMAN_W),
	)
	
	DESCRIPTION_FILE_NAME = (
		uiScriptLocale.JOBDESC_WARRIOR_PATH,
		uiScriptLocale.JOBDESC_ASSASSIN_PATH,
		uiScriptLocale.JOBDESC_SURA_PATH,
		uiScriptLocale.JOBDESC_SHAMAN_PATH,
	)
	
	RACE_FACE_PATH = {
		playerSettingModule.RACE_WARRIOR_M : "D:/ymir work/ui/intro/public_intro/face/face_warrior_m_0",
		playerSettingModule.RACE_ASSASSIN_W : "D:/ymir work/ui/intro/public_intro/face/face_assassin_w_0",
		playerSettingModule.RACE_SURA_M : "D:/ymir work/ui/intro/public_intro/face/face_sura_m_0",
		playerSettingModule.RACE_SHAMAN_W : "D:/ymir work/ui/intro/public_intro/face/face_shaman_w_0",
		playerSettingModule.RACE_WARRIOR_W : "D:/ymir work/ui/intro/public_intro/face/face_warrior_w_0",
		playerSettingModule.RACE_ASSASSIN_M : "D:/ymir work/ui/intro/public_intro/face/face_assassin_m_0",
		playerSettingModule.RACE_SURA_W : "D:/ymir work/ui/intro/public_intro/face/face_sura_w_0",
		playerSettingModule.RACE_SHAMAN_M : "D:/ymir work/ui/intro/public_intro/face/face_shaman_m_0",
	}
	
	DISC_FACE_PATH = {
		playerSettingModule.RACE_WARRIOR_M :"d:/ymir work/bin/icon/face/warrior_m.tga",
		playerSettingModule.RACE_ASSASSIN_W :"d:/ymir work/bin/icon/face/assassin_w.tga",
		playerSettingModule.RACE_SURA_M :"d:/ymir work/bin/icon/face/sura_m.tga",
		playerSettingModule.RACE_SHAMAN_W :"d:/ymir work/bin/icon/face/shaman_w.tga",
		playerSettingModule.RACE_WARRIOR_W :"d:/ymir work/bin/icon/face/warrior_w.tga",
		playerSettingModule.RACE_ASSASSIN_M :"d:/ymir work/bin/icon/face/assassin_m.tga",
		playerSettingModule.RACE_SURA_W :"d:/ymir work/bin/icon/face/sura_w.tga",
		playerSettingModule.RACE_SHAMAN_M :"d:/ymir work/bin/icon/face/shaman_m.tga",
	}

	LEN_JOBLIST = len(M2JOBLIST)
	LEN_STATPOINT = len(M2STATPOINT[0])
	M2_INIT_VALUE = -1

	class DescriptionBox(ui.Window):
		def __init__(self):
			ui.Window.__init__(self)
			self.descIndex = 0

		def SetIndex(self, index):
			self.descIndex = index

		def OnRender(self):
			event.RenderEventSet(self.descIndex)

	class CharacterRenderer(ui.Window):
		def __init__(self):
			ui.Window.__init__(self)
			self.screenWidth = wndMgr.GetScreenWidth()
			self.screenHeight = wndMgr.GetScreenHeight()
			self.newScreenWidth = float(self.screenWidth)
			self.newScreenHeight = float(self.screenHeight)
			self.viewportWidth = self.newScreenWidth / self.screenWidth
			self.viewportHeight = self.newScreenHeight / self.screenHeight

		def setCameraSettings(self):
			app.SetCenterPosition(0.0, 0.0, 0.0)
			app.SetCamera(1550.0, 15.0, 180.0, 95.0)
			grp.SetPerspective(10.0, self.newScreenWidth / self.newScreenHeight, 1000.0, 3000.0)

		def OnRender(self):
			grp.ClearDepthBuffer()
			grp.SetGameRenderState()
			grp.PushState()
			grp.SetOmniLight()

			grp.SetViewport(0.0, 0.0, self.viewportWidth, self.viewportHeight)

			self.setCameraSettings()

			(x, y) = app.GetCursorPosition()
			grp.SetCursorPosition(x, y)

			chr.Deform()
			chr.Render()

			grp.RestoreViewport()
			grp.PopState()
			grp.SetInterfaceRenderState()

	def __init__(self, stream):
		ui.Window.__init__(self)
		net.SetPhaseWindow(net.PHASE_WINDOW_CREATE, self)
		self.stream = stream
		self.initialize_values()

		
	def initialize_values(self):
		self.ShowToolTip = False
		self.createSuccess = False
		self.MotionStart = False
		self.MotionTime = 0.0
		self.gender = self.M2_INIT_VALUE
		b_id = self.M2_INIT_VALUE
		self.shape = self.M2_INIT_VALUE
		self.descIndex = 0
		self.statpoint = [0, 0, 0, 0]
		self.curGauge = [0.0, 0.0, 0.0, 0.0]

	def __del__(self):
		net.SetPhaseWindow(net.PHASE_WINDOW_CREATE, 0)
		ui.Window.__del__(self)

	def Open(self):
		playerSettingModule.LoadGameData("INIT")
		dlgBoard = ui.ScriptWindow()
		self.dlgBoard = dlgBoard
		pythonScriptLoader = ui.PythonScriptLoader()
		pythonScriptLoader.LoadScriptFile( self.dlgBoard, uiScriptLocale.LOCALE_UISCRIPT_PATH + "createcharacterwindow.py" ) 
		getChild = self.dlgBoard.GetChild

		self.NameList = [getChild("name_%s" % job) for job in ["warrior", "assassin", "sura", "shaman"]]

		self.textBoard = getChild("text_board")
		self.btnPrev = getChild("prev_button")
		self.btnNext = getChild("next_button")

		self.JobList = [getChild(job.upper()) for job in ["warrior", "assassin", "sura", "shaman"]]

		self.SelectBtnNameList = [getChild("name_%s_small" % job) for job in ["warrior", "assassin", "sura", "shaman"]]
		self.SelectBtnFaceList = [getChild("CharacterFace_%d" % i) for i in range(4)]
		self.genderButtonList = [getChild("gender_button_0%d" % i) for i in range(1, 3)]
		self.shapeButtonList = [getChild("shape_button_0%d" % i) for i in range(1, 3)]
		self.statValue = [getChild("%s_value" % stat) for stat in ["hth", "int", "str", "dex"]]
		self.GaugeList = [getChild("%s_gauge" % stat) for stat in ["hth", "int", "str", "dex"]]

		self.btnCreate = getChild("create_button")
		self.btnCancel = getChild("cancel_button")

		self.empireName = getChild("EmpireName")
		self.flagDict = {
			net.EMPIRE_B : "d:/ymir work/ui/intro/empire/empireflag_b.sub",
			net.EMPIRE_C : "d:/ymir work/ui/intro/empire/empireflag_c.sub",
		}
		self.flag = getChild("EmpireFlag")

		self.discFace = getChild("DiscFace")
		self.raceNameText = getChild("raceName_Text")
		self.editCharacterName = getChild("character_name_value")

		self.backGroundDict = {
			net.EMPIRE_B : "d:/ymir work/ui/intro/empire/background/empire_chunjo.sub",
			net.EMPIRE_C : "d:/ymir work/ui/intro/empire/background/empire_jinno.sub",
		}

		self.JobList[0].SetEvent(ui.__mem_func__(self.SelectJob), 0)
		self.JobList[1].SetEvent(ui.__mem_func__(self.SelectJob), 1)
		self.JobList[2].SetEvent(ui.__mem_func__(self.SelectJob), 2)
		self.JobList[3].SetEvent(ui.__mem_func__(self.SelectJob), 3)
		
		self.btnCreate.SetEvent(ui.__mem_func__(self.CreateCharacterButton))
		self.btnCancel.SetEvent(ui.__mem_func__(self.CancelButton))

		self.btnPrev.SetEvent(ui.__mem_func__(self.PrevDescriptionPage))
		self.btnNext.SetEvent(ui.__mem_func__(self.NextDescriptionPage))

		self.editCharacterName.SetReturnEvent(ui.__mem_func__(self.CreateCharacterButton))
		self.editCharacterName.SetEscapeEvent(ui.__mem_func__(self.CancelButton))

		for i in range(4):
			self.JobList[i].SetEvent(ui.__mem_func__(self.SelectJob), i)
			self.SelectBtnNameList[i].SetEvent(ui.__mem_func__(self.EventProgress), "mouse_click", i)
			self.SelectBtnNameList[i].SetEvent(ui.__mem_func__(self.EventProgress), "mouse_over_in", i)
			self.SelectBtnNameList[i].SetEvent(ui.__mem_func__(self.EventProgress), "mouse_over_out", i)
			self.SelectBtnFaceList[i].SetEvent(ui.__mem_func__(self.EventProgress), "mouse_click", i)
			self.SelectBtnFaceList[i].SetEvent(ui.__mem_func__(self.EventProgress), "mouse_over_in", i)
			self.SelectBtnFaceList[i].SetEvent(ui.__mem_func__(self.EventProgress), "mouse_over_out", i)
			
		self.genderButtonList[MAN].SetEvent(ui.__mem_func__(self.SelectGender), MAN)
		self.genderButtonList[WOMAN].SetEvent(ui.__mem_func__(self.SelectGender), WOMAN)
		
		for i in range(2):
			self.shapeButtonList[i].SetEvent(ui.__mem_func__(self.SelectShape), i)
		
		tool_tip_mappings = {
			"WARRIOR": self.M2JOBLIST[0],
			"ASSASSIN": self.M2JOBLIST[1],
			"SURA": self.M2JOBLIST[2],
			"SHAMAN": self.M2JOBLIST[3],
			"gender_button_01": uiScriptLocale.CREATE_MAN,
			"gender_button_02": uiScriptLocale.CREATE_WOMAN,
			"shape_button_01": uiScriptLocale.CREATE_SHAPE + "1",
			"shape_button_02": uiScriptLocale.CREATE_SHAPE + "2",
			"prev_button": uiScriptLocale.CREATE_PREV,
			"next_button": uiScriptLocale.CREATE_NEXT,
			"create_button": uiScriptLocale.CREATE_CREATE,
			"cancel_button": uiScriptLocale.CANCEL
		}

		for name, tooltip_arg in tool_tip_mappings.items():
			child = getChild(name)
			child.ShowToolTip = lambda arg = tooltip_arg: self.OverInToolTip(arg)
			child.HideToolTip = self.OverOutToolTip
		
		self.backGround = getChild("BackGround")
		self.JobSex = {i: app.GetRandom(MAN, WOMAN) for i in range(4)}
		self.toolTip = uiToolTip.ToolTip()
		self.toolTip.ClearToolTip()
		self.editCharacterName.SetText("")
		self.EnableWindow()
		
		my_empire = net.GetEmpireID()
		self.SetEmpire(my_empire)
		self.descriptionBox = self.DescriptionBox()
		self.descriptionBox.Show()
		self.chrRenderer = self.CharacterRenderer()
		self.chrRenderer.SetParent(self.backGround)
		self.chrRenderer.Show()

		for race in [playerSettingModule.RACE_WARRIOR_M, playerSettingModule.RACE_ASSASSIN_M, playerSettingModule.RACE_SURA_M, playerSettingModule.RACE_SHAMAN_M, playerSettingModule.RACE_WARRIOR_W, playerSettingModule.RACE_ASSASSIN_W, playerSettingModule.RACE_SURA_W, playerSettingModule.RACE_SHAMAN_W]:
			self.MakeCharacter(race)

		self.dlgBoard.Show()
		self.Show()
		app.ShowCursor()

		if musicInfo.createMusic != "":
			snd.SetMusicVolume(systemSetting.GetMusicVolume())
			snd.FadeInMusic("BGM/"+musicInfo.createMusic)

		self.SelectJob(app.GetRandom(0, self.LEN_JOBLIST - 1))

	def Close(self):
		attributes_to_reset = [
			"ShowToolTip", "createSuccess", "MotionStart", "MotionTime", "gender", "job_id", "shape", "descIndex",
			"statpoint", "curGauge", "stream", "M2STATPOINT", "M2JOBLIST", "M2_CONST_ID", "LEN_JOBLIST", "LEN_STATPOINT",
			"M2_INIT_VALUE", "EMPIRE_NAME", "EMPIRE_NAME_COLOR", "DESCRIPTION_FILE_NAME", "RACE_FACE_PATH", "DISC_FACE_PATH",
			"NameList", "JobList", "SelectBtnNameList", "SelectBtnFaceList", "genderButtonList", "shapeButtonList",
			"statValue", "GaugeList", "flag", "flagDict", "raceNameText", "textBoard", "btnPrev", "btnNext", "btnCreate",
			"btnCancel", "empireName", "editCharacterName", "backGround", "backGroundDict", "toolTip", "JobSex"
		]

		for i in xrange(self.LEN_JOBLIST * 2):
			chr.DeleteInstance(i)

		for attr in attributes_to_reset:
			setattr(self, attr, None)

		self.dlgBoard.ClearDictionary()

		if musicInfo.createMusic:
			snd.FadeOutMusic("BGM/" + musicInfo.createMusic)

		self.dlgBoard.Hide()
		self.Hide()
		app.HideCursor()
		event.Destroy()

	def OnCreateSuccess(self):
		self.createSuccess = True

	def OnCreateFailure(self, type):
		self.MotionStart = False
		chr.BlendLoopMotion(chr.MOTION_INTRO_WAIT, 0.1)

		error_messages = {
			1: localeInfo.CREATE_EXIST_SAME_NAME,
			2: localeInfo.CREATE_ERROR_LIMIT_LEVEL,
			3: localeInfo.CREATE_ERROR_TIME_LIMIT,
			4: localeInfo.CREATE_ERROR_WRONG_STRING,
			5: localeInfo.CREATE_CHARACTER_DISABLED,
			6: localeInfo.CREATE_ERROR_LIMIT_CHARACTER,
			7: localeInfo.CREATE_LOGIN_TO_NAME,
			8: localeInfo.CREATE_NAME_TO_COMPICATE,			
		}

		message = error_messages.get(type, localeInfo.CREATE_FAILURE)
		self.PopupMessage(message, self.EnableWindow)

		return self.OverOutToolTip()

	def EnableWindow(self):
		self.btnPrev.Enable()
		self.btnNext.Enable()
		self.btnCreate.Enable()
		self.btnCancel.Enable()
		self.editCharacterName.SetFocus()
		self.editCharacterName.Enable()

		for i in xrange(2):
			self.genderButtonList[i].Enable()
			self.shapeButtonList[i].Enable()

	def DisableWindow(self):
		self.OverOutToolTip()

		self.btnPrev.Disable()
		self.btnNext.Disable()
		self.btnCreate.Disable()
		self.btnCancel.Disable()
		self.editCharacterName.Disable()

		for i in xrange(2):
			self.genderButtonList[i].Disable()
			self.shapeButtonList[i].Disable()

	def MakeCharacter(self, race):
		chr_id = race

		chr.CreateInstance(chr_id)
		chr.SelectInstance(chr_id)
		chr.SetVirtualID(chr_id)
		chr.SetNameString(str(race))

		chr.SetRace(race)
		chr.SetArmor(0)
		chr.SetHair(0)

		chr.SetMotionMode(chr.MOTION_MODE_GENERAL)
		chr.SetLoopMotion(chr.MOTION_INTRO_WAIT)

		chr.SetRotation(-7.0)
		chr.Hide()

	def SelectJob(self, job_id):
		if self.MotionStart:
			self.JobList[job_id].SetUp()
			return

		for button in self.JobList:
			button.SetUp()

		self.job_id = job_id
		self.JobList[self.job_id].Down()
		self.SelectGender(self.JobSex[job_id])

		event.ClearEventSet(self.descIndex)
		self.descIndex = event.RegisterEventSet(self.DESCRIPTION_FILE_NAME[self.job_id])
		event.SetFontColor(self.descIndex, 0.7843, 0.7843, 0.7843)
		total_line = event.GetTotalLineCount(self.descIndex)

		self.raceNameText.SetText(self.M2JOBLIST[job_id])

		if localeInfo.IsARABIC():
			event.SetEventSetWidth(self.descIndex, 170)
		else:
			event.SetRestrictedCount(self.descIndex, 35)

		if event.BOX_VISIBLE_LINE_COUNT >= total_line:
			self.btnPrev.Hide()
			self.btnNext.Hide()
		else:
			self.btnPrev.Show()
			self.btnNext.Show()

		self.ResetStat()
		self.genderButtonList[WOMAN].Show()

	def SelectGender(self, gender):
		for button in self.genderButtonList:
			button.SetUp()

		self.gender = gender
		self.genderButtonList[self.gender].Down()

		if self.M2_INIT_VALUE == self.job_id:
			return

		self.JobSex[self.job_id] = self.gender
		self.race = self.M2_CONST_ID[self.job_id][self.gender]

		for i in xrange(self.LEN_JOBLIST):
			if self.job_id == i:
				self.SelectBtnFaceList[i].LoadImage(self.RACE_FACE_PATH[self.race] + "1.sub")
			else:
				self.SelectBtnFaceList[i].LoadImage(self.RACE_FACE_PATH[self.M2_CONST_ID[i][self.JobSex[i]]] + "2.sub")

		self.discFace.LoadImage(self.DISC_FACE_PATH[self.race])

		if self.M2_INIT_VALUE == self.shape:
			self.shape = 0

		self.SelectShape(self.shape)

	def SelectShape(self, shape):
		for button in self.shapeButtonList:
			button.SetUp()

		self.shape = shape
		self.shapeButtonList[self.shape].Down()

		if self.M2_INIT_VALUE == self.job_id:
			return

		chr.Hide()
		chr.SelectInstance(self.race)
		chr.ChangeShape(self.shape)
		chr.SetMotionMode(chr.MOTION_MODE_GENERAL)
		chr.SetLoopMotion(chr.MOTION_INTRO_WAIT)
		chr.Show()

	def RefreshStat(self):
		statSummary = 8.0
		self.curGauge = [
			float(self.statpoint[0]) / statSummary,
			float(self.statpoint[1]) / statSummary,
			float(self.statpoint[2]) / statSummary,
			float(self.statpoint[3]) / statSummary,
		]

		for i in xrange(self.LEN_STATPOINT):
			self.statValue[i].SetText(str(self.statpoint[i]))

	def ResetStat(self):
		for i in xrange(self.LEN_STATPOINT):
			self.statpoint[i] = self.M2STATPOINT[self.job_id][i]
		self.RefreshStat()

	def CreateCharacterButton(self):
		if self.job_id == self.M2_INIT_VALUE or self.MotionStart:
			return

		textName = self.editCharacterName.GetText()

		if False == self.CheckCreateCharacterName(textName):
			return

		self.DisableWindow()

		chrSlot = self.stream.GetCharacterSlot()
		raceIndex = self.M2_CONST_ID[self.job_id][self.gender]
		shapeIndex = self.shape
		statCon = self.M2STATPOINT[self.job_id][0]
		statInt = self.M2STATPOINT[self.job_id][1]
		statStr = self.M2STATPOINT[self.job_id][2]
		statDex = self.M2STATPOINT[self.job_id][3]

		chr.PushOnceMotion(chr.MOTION_INTRO_SELECTED)
		net.SendCreateCharacterPacket(chrSlot, textName, raceIndex, shapeIndex, statCon, statInt, statStr, statDex)

		self.MotionStart = True
		self.MotionTime = app.GetTime()

	def CancelButton(self):
		self.stream.SetSelectCharacterPhase()
		self.Hide()

	def EmptyFunc(self):
		pass

	def PopupMessage(self, msg, func = 0):
		func = func or self.EmptyFunc
		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(msg, func, localeInfo.UI_OK)

	def OnPressExitKey(self):
		self.CancelButton()
		return True

	def CheckCreateCharacterName(self, name):
		if not name:
			self.PopupMessage(localeInfo.CREATE_INPUT_NAME, self.EnableWindow)
			return False

		if localeInfo.CREATE_GM_NAME in name:
			self.PopupMessage(localeInfo.CREATE_ERROR_GM_NAME, self.EnableWindow)
			return False

		return True

	def SetEmpire(self, empire_id):
		self.empireName.SetText(self.EMPIRE_NAME.get(empire_id, ""))
		rgb = self.EMPIRE_NAME_COLOR[empire_id]
		self.empireName.SetFontColor(*rgb)
		if empire_id != net.EMPIRE_A:
			self.flag.LoadImage(self.flagDict[empire_id])
			self.flag.SetScale(0.45, 0.45)
			self.backGround.LoadImage(self.backGroundDict[empire_id])
			self.backGround.SetScale(float(wndMgr.GetScreenWidth()) / 1024.0, float(wndMgr.GetScreenHeight()) / 768.0)

	def PrevDescriptionPage(self):
		if event.IsWait(self.descIndex):
			start_line = event.GetVisibleStartLine(self.descIndex) - event.BOX_VISIBLE_LINE_COUNT
			if start_line >= 0:
				event.SetVisibleStartLine(self.descIndex, start_line)
		event.Skip(self.descIndex)

	def NextDescriptionPage(self):
		if event.IsWait(self.descIndex):
			event.SetVisibleStartLine(self.descIndex, event.GetVisibleStartLine(self.descIndex) + event.BOX_VISIBLE_LINE_COUNT)
		event.Skip(self.descIndex)

	def EventProgress(self, event_type, slot):
		if event_type == "mouse_click" and slot != self.job_id:
			snd.PlaySound("sound/ui/click.wav")
			self.SelectJob(slot)
		elif event_type == "mouse_over_in":
			self.JobList[self.job_id].Down()
			self.OverInToolTip(self.M2JOBLIST[slot])
		elif event_type == "mouse_over_out":
			self.JobList[self.job_id].Down()
			self.OverOutToolTip()

	def OverInToolTip(self, arg):
		pos_x, pos_y = wndMgr.GetMousePosition()
		self.toolTip.ClearToolTip()
		self.toolTip.SetThinBoardSize(11 * len(str(arg)))
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

	def OnUpdate(self):
		chr.Update()
		self.ToolTipProgress()
		xposEventSet, yposEventSet = self.textBoard.GetGlobalPosition()
		event.UpdateEventSet(self.descIndex, xposEventSet+7, -(yposEventSet+7))
		self.descriptionBox.SetIndex(self.descIndex)
		for i, name in enumerate(self.NameList):
			name.SetAlpha(1 if self.job_id == i else 0)
		for i, gauge in enumerate(self.GaugeList):
			gauge.SetPercentage(self.curGauge[i], 1.0)
		if self.MotionStart and self.createSuccess and app.GetTime() - self.MotionTime >= 2.0:
			self.MotionStart = False
			self.stream.SetSelectCharacterPhase()
			self.Hide()
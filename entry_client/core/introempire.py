# -----------------------------------------------------
# Title: Zakodowane na nowo intropage like official
# Author: Sevence
# Website: Sharegon.pl
# Date: 13.09.2023r.
# -----------------------------------------------------

import ui
import app
import dbg
import net
import snd
import event
import wndMgr
import _weakref
import musicInfo
import uiToolTip
import exception
import localeInfo
import systemSetting
import uiScriptLocale

class SelectEmpireWindow(ui.ScriptWindow):
	WINDOW_NAME = "SelectEmpireWindow"
	SCRIPT_PATH = uiScriptLocale.LOCALE_UISCRIPT_PATH + "SelectEmpireWindow.py"
	
	EMPIRES = {
		net.EMPIRE_A: {
			"name": localeInfo.EMPIRE_A,
			"color": (0.7450, 0, 0),
			"desc_file": uiScriptLocale.EMPIREDESC_A
		},
		net.EMPIRE_B: {
			"name": localeInfo.EMPIRE_B,
			"color": (0.8666, 0.6156, 0.1843),
			"desc_file": uiScriptLocale.EMPIREDESC_B
		},
		net.EMPIRE_C: {
			"name": localeInfo.EMPIRE_C,
			"color": (0.2235, 0.2549, 0.7490),
			"desc_file": uiScriptLocale.EMPIREDESC_C
		}
	}

	class EmpireButton(ui.Window):
		TEXT_MAP = {
			net.EMPIRE_A: localeInfo.EMPIRE_A,
			net.EMPIRE_B: localeInfo.EMPIRE_B,
			net.EMPIRE_C: localeInfo.EMPIRE_C
		}
	
		def __init__(self, owner, arg):
			ui.Window.__init__(self)
			self.owner = owner
			self.arg = arg			
			
		def OnMouseOverIn(self):
			self.owner.OnOverInEmpire(self.arg)
			
			if self.arg not in self.TEXT_MAP:
				raise ValueError("Invalid Empire Button argument: {}".format(self.arg))

		def OnMouseOverOut(self):
			self.owner.OnOverOutEmpire(self.arg)
			self.owner.OverOutToolTip()

		def OnMouseLeftButtonDown(self):
			if self.owner.empireID != self.arg:
				self.owner.OnSelectEmpire(self.arg)

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

	def __init__(self, stream):
		ui.ScriptWindow.__init__(self)
		net.SetPhaseWindow(net.PHASE_WINDOW_EMPIRE, self)

		self.stream = stream
		self.empireID = net.GetEmpireID()
		self.descIndex = 0
		
		self.empireArea = {}
		self.empireAreaFlag = {}
		self.empireFlag = {}
		self.empireAreaButton = {}
		
		self.empireAreaCurAlpha = self._generate_initial_alpha_values()
		self.empireAreaDestAlpha = self._generate_initial_alpha_values()
		self.empireAreaFlagCurAlpha = self._generate_initial_alpha_values()
		self.empireAreaFlagDestAlpha = self._generate_initial_alpha_values()
		self.empireFlagCurAlpha = self._generate_initial_alpha_values()
		self.empireFlagDestAlpha = self._generate_initial_alpha_values()
	
	@staticmethod
	def _generate_initial_alpha_values():
		return {net.EMPIRE_A: 0.0, net.EMPIRE_B: 0.0, net.EMPIRE_C: 0.0}		

	def __del__(self):
		ui.ScriptWindow.__del__(self)
		net.SetPhaseWindow(net.PHASE_WINDOW_EMPIRE, 0)
		
		self.stream = None
		self.empireID = None
		self.descIndex = None
		self.empireArea = None
		self.empireAreaFlag = None
		self.empireFlag = None
		self.empireAreaButton = None
    
		self.empireAreaCurAlpha = None
		self.empireAreaDestAlpha = None
		self.empireAreaFlagCurAlpha = None
		self.empireAreaFlagDestAlpha = None
		self.empireFlagCurAlpha = None
		self.empireFlagDestAlpha = None
		self.toolTip = None
		self.ShowToolTip = None

	def Close(self):
		self.ClearDictionary()
		
		self.empireArea.clear()
		self.empireAreaFlag.clear()
		self.empireFlag.clear()
		self.empireAreaButton.clear()		
		
		attributes_to_clear = [
			"leftButton", 
			"rightButton", 
			"selectButton", 
			"exitButton",
			"textBoard", 
			"descriptionBox", 
			"empireArea", 
			"empireAreaButton",
			"empireName", 
			"toolTip", 
			"ShowToolTip", 
			"btnPrev", 
			"btnNext"
		]

		for attribute in attributes_to_clear:
			attr_value = getattr(self, attribute, None)
			if isinstance(attr_value, dict):
				for key in attr_value.keys():
					attr_value[key] = None
				attr_value.clear()
			setattr(self, attribute, None)
		
		if musicInfo.selectMusic != "":
			snd.FadeOutMusic("BGM/"+musicInfo.selectMusic)		

		self.KillFocus()
		self.Hide()
		app.HideCursor()
		event.Destroy()

	def Open(self):
		self._setup_window()
		self._load_and_setup_script()
		self._play_music()
		self._setup_tooltip()
		
	def _setup_window(self):
		self.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		self.SetWindowName(self.WINDOW_NAME)
		self.Show()

	def _load_and_setup_script(self):
		if not self.__LoadScript(self.SCRIPT_PATH):
			dbg.TraceError("SelectEmpireWindow.Open - __LoadScript Error")
			return
		self.OnSelectEmpire(self.empireID)
		self.__CreateButtons()
		self.__CreateDescriptionBox()
		app.ShowCursor()

	def _play_music(self):
		if musicInfo.selectMusic:
			snd.SetMusicVolume(systemSetting.GetMusicVolume())
			snd.FadeInMusic("BGM/" + musicInfo.selectMusic)

	def _setup_tooltip(self):
		self.toolTip = uiToolTip.ToolTip()
		self.toolTip.ClearToolTip()
		self.ShowToolTip = False		

	def __CreateButtons(self):
		for key, img in self.empireArea.items():

			img.SetAlpha(0.0)
			(x, y) = img.GetGlobalPosition()

			btn = self.EmpireButton(_weakref.proxy(self), key)
			btn.SetParent(self)
			btn.SetPosition(x, y)
			btn.SetSize(img.GetWidth(), img.GetHeight())
			btn.Show()

			self.empireAreaButton[key] = btn

	def __CreateDescriptionBox(self):
		self.descriptionBox = self.DescriptionBox()
		self.descriptionBox.Show()

	def OnOverInEmpire(self, arg):
		self.empireAreaDestAlpha[arg] = 1.0

	def OnOverOutEmpire(self, arg):
		if arg != self.empireID:
			self.empireAreaDestAlpha[arg] = 0.0

	def OnSelectEmpire(self, arg):
		for key in self.empireArea.keys():
			self.empireAreaDestAlpha[key] = 0.0
			self.empireAreaFlagDestAlpha[key] = 0.0
			self.empireFlagDestAlpha[key] = 0.0
			
		self.empireAreaDestAlpha[arg] = 1.0
		self.empireAreaFlagDestAlpha[arg] = 1.0
		self.empireFlagDestAlpha[arg] = 1.0
		self.empireID = arg
	
		empire_data = self.EMPIRES.get(self.empireID, {})
		self.empireName.SetText(empire_data.get("name", ""))
		rgb = empire_data.get("color", (1, 1, 1))
		self.empireName.SetFontColor(*rgb)
		snd.PlaySound("sound/ui/click.wav")

		event.ClearEventSet(self.descIndex)
				
		desc_file = empire_data.get("desc_file")
		if desc_file:
			self.descIndex = event.RegisterEventSet(desc_file)
			event.SetFontColor(self.descIndex, 0.7843, 0.7843, 0.7843)				

		event.SetRestrictedCount(self.descIndex, 35)
		
		total_line_count = event.GetTotalLineCount(self.descIndex)
		if event.BOX_VISIBLE_LINE_COUNT >= total_line_count:
			self.btnPrev.Hide()
			self.btnNext.Hide()
		else:
			self.btnPrev.Show()
			self.btnNext.Show()

	def PrevDescriptionPage(self):
		if True == event.IsWait(self.descIndex):
			if event.GetVisibleStartLine(self.descIndex) - event.BOX_VISIBLE_LINE_COUNT >= 0:
				event.SetVisibleStartLine(self.descIndex, event.GetVisibleStartLine(self.descIndex) - event.BOX_VISIBLE_LINE_COUNT)
				event.Skip(self.descIndex)
		else:
			event.Skip(self.descIndex)

	def NextDescriptionPage(self):
		if True == event.IsWait(self.descIndex):
			event.SetVisibleStartLine(self.descIndex, event.GetVisibleStartLine(self.descIndex) + event.BOX_VISIBLE_LINE_COUNT)
			event.Skip(self.descIndex)
		else:
			event.Skip(self.descIndex)

	def __LoadScript(self, fileName):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, fileName)
		except:
			exception.Abort("SelectEmpireWindow.__LoadScript.LoadObject")

		GetObject = self.GetChild

		attributes = {
			"leftButton": "left_button",
			"rightButton": "right_button",
			"selectButton": "select_button",
			"exitButton": "exit_button",
			"textBoard": "text_board",
			"empireName": "EmpireName",
			"btnPrev": "prev_text_button",
			"btnNext": "next_text_button",
		}

		for key, value in attributes.items():
			setattr(self, key, GetObject(value))

		self.empireArea = {
			net.EMPIRE_A: GetObject("EmpireArea_A"),
			net.EMPIRE_B: GetObject("EmpireArea_B"),
			net.EMPIRE_C: GetObject("EmpireArea_C"),
		}

		self.empireAreaFlag = {
			net.EMPIRE_A: GetObject("EmpireAreaFlag_A"),
			net.EMPIRE_B: GetObject("EmpireAreaFlag_B"),
			net.EMPIRE_C: GetObject("EmpireAreaFlag_C"),
		}

		self.empireFlag = {
			net.EMPIRE_A: GetObject("EmpireFlag_A"),
			net.EMPIRE_B: GetObject("EmpireFlag_B"),
			net.EMPIRE_C: GetObject("EmpireFlag_C"),
		}

		for button_name, tooltip_text in [
			("left_button", uiScriptLocale.EMPIRE_PREV),
			("right_button", uiScriptLocale.EMPIRE_NEXT),
			("prev_text_button", uiScriptLocale.EMPIRE_PREV),
			("next_text_button", uiScriptLocale.EMPIRE_NEXT),
			("select_button", uiScriptLocale.EMPIRE_SELECT),
			("exit_button", uiScriptLocale.EMPIRE_EXIT),
		]:
		
			btn = GetObject(button_name)
			self_ref = _weakref.ref(self)
			btn.ShowToolTip = lambda arg=tooltip_text: self_ref().OverInToolTip(arg) if self_ref() is not None else None
			btn.HideToolTip = self.OverOutToolTip

		try:
			GetObject("prev_text_button").SetEvent(ui.__mem_func__(self.PrevDescriptionPage))
			GetObject("next_text_button").SetEvent(ui.__mem_func__(self.NextDescriptionPage))

			self.selectButton.SetEvent(ui.__mem_func__(self.ClickSelectButton))
			self.exitButton.SetEvent(ui.__mem_func__(self.ClickExitButton))
			self.leftButton.SetEvent(ui.__mem_func__(self.ClickRightButton))
			self.rightButton.SetEvent(ui.__mem_func__(self.ClickLeftButton))

			for flag in self.empireAreaFlag.values():
				flag.SetAlpha(0.0)
			for flag in self.empireFlag.values():
				flag.SetAlpha(0.0)
		except:
			exception.Abort("SelectEmpireWindow.__LoadScript.BindObject")

		return 1

	def ClickLeftButton(self):
		self.ChangeEmpireID(-1)
		self.OnSelectEmpire(self.empireID)

	def ClickRightButton(self):
		self.ChangeEmpireID(1)
		self.OnSelectEmpire(self.empireID)

	def ChangeEmpireID(self, change):
		self.empireID += change
		if self.empireID < 1:
			self.empireID = 3
		elif self.empireID > 3:
			self.empireID = 1

	def ClickSelectButton(self):
		net.SendSelectEmpirePacket(self.empireID)
		self.stream.SetSelectCharacterPhase()
		self.Hide()

	def ClickExitButton(self):
		self.stream.SetLoginPhase()
		self.Hide()

	def OnUpdate(self):
		self._updateEventSetPosition()
		self._updateDescriptionBoxIndex()

		attributes_to_update = [
			(self.empireArea, self.empireAreaCurAlpha, self.empireAreaDestAlpha),
			(self.empireAreaFlag, self.empireAreaFlagCurAlpha, self.empireAreaFlagDestAlpha),
			(self.empireFlag, self.empireFlagCurAlpha, self.empireFlagDestAlpha)
		]

		for attr in attributes_to_update:
			self.__UpdateAlpha(*attr)

		self.ToolTipProgress()
		
	def _updateEventSetPosition(self):
		xposEventSet, yposEventSet = self.textBoard.GetGlobalPosition()
		event.UpdateEventSet(self.descIndex, xposEventSet + 7, -(yposEventSet + 7))

	def _updateDescriptionBoxIndex(self):
		self.descriptionBox.SetIndex(self.descIndex)		

	def __UpdateAlpha(self, dict, curAlphaDict, destAlphaDict):
		for key, img in dict.items():

			curAlpha = curAlphaDict[key]
			destAlpha = destAlphaDict[key]

			if abs(destAlpha - curAlpha) / 10 > 0.0001:
				curAlpha += (destAlpha - curAlpha) / 7
			else:
				curAlpha = destAlpha

			curAlphaDict[key] = curAlpha
			img.SetAlpha(curAlpha)

	def OnPressEscapeKey(self):
		self.ClickExitButton()
		return True

	def OverInToolTip(self, arg) :	
		arglen = len(str(arg))
		pos_x, pos_y = wndMgr.GetMousePosition()
			
		self.toolTip.ClearToolTip()
		self.toolTip.SetThinBoardSize(11 * arglen)
		self.toolTip.SetToolTipPosition(pos_x + 50, pos_y + 50)
		self.toolTip.AppendTextLine(str(arg), 0xffffff00)
		self.toolTip.Show()
		self.ShowToolTip = True
	
	def OverOutToolTip(self):
		self.toolTip.Hide()
		self.ShowToolTip = False
			
	def ToolTipProgress(self):
		if self.ShowToolTip:
			pos_x, pos_y = wndMgr.GetMousePosition()
			self.toolTip.SetToolTipPosition(pos_x + 50, pos_y + 50)
			
	def OnPressExitKey(self):
		self.stream.SetLoginPhase()
		self.Hide()
		return True

class ReselectEmpireWindow(SelectEmpireWindow):
	def ClickSelectButton(self):
		net.SendSelectEmpirePacket(self.empireID)
		self.stream.SetCreateCharacterPhase()
		self.Hide()

	def ClickExitButton(self):
		self.stream.SetLoginPhase()
		self.Hide()
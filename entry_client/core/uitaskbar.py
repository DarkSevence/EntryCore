import os
import ui
import app
import item
import skill
import wndMgr
import player
import constInfo
import localeInfo
import mouseModule
import uiScriptLocale

class MouseSettings:
	def __init__(self):
		self.settings = [0, 0]

	def InitMouseButtonSettings(self, left, right):
		self.settings = [left, right]

	def SetMouseButtonSetting(self, button, event):
		if button < 0 or button >= len(self.settings):
			sys_err("Invalid button index")

		self.settings[button] = event

	def GetMouseButtonSettings(self):
		return self.settings

	def SaveMouseButtonSettings(self, filepath="config/mouse.cfg"):
		with open(filepath, "w") as file:
			file.write("%d\t%d" % tuple(self.settings))

	def LoadMouseButtonSettings(self, filepath="config/mouse.cfg"):
		if not os.path.exists(filepath):
			sys_err("MOUSE_SETTINGS_FILE_ERROR: File does not exist")

		with open(filepath, "r") as file:
			tokens = file.read().split()

		if len(tokens) != 2:
			sys_err("MOUSE_SETTINGS_FILE_ERROR: Invalid file format")

		self.settings = [int(tokens[0]), int(tokens[1])]

	@staticmethod
	def unsigned32(n):
		return n & 0xFFFFFFFFL

class GiftBox(ui.ScriptWindow):
	class TextToolTip(ui.Window):
		def __init__(self):
			super(GiftBox.TextToolTip, self).__init__("TOP_MOST")
			self.SetWindowName("GiftBox")
			self.textLine = ui.TextLine()
			self.textLine.SetParent(self)
			self.textLine.SetHorizontalAlignCenter()
			self.textLine.SetOutline()
			self.textLine.Show()

		def __del__(self):
			self.textLine = None
			super(GiftBox.TextToolTip, self).__del__()

		def SetText(self, text):
			self.textLine.SetText(text)

		def OnRender(self):
			(mouseX, mouseY) = wndMgr.GetMousePosition()
			self.textLine.SetPosition(mouseX, mouseY - 15)

	def __init__(self):
		super(GiftBox, self).__init__()
		self.tooltipGift = self.TextToolTip()
		self.tooltipGift.Show()

	def __del__(self):
		self.tooltipGift = None
		super(GiftBox, self).__del__()

	def LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, uiScriptLocale.LOCALE_UISCRIPT_PATH + "giftbox.py")
		except Exception as e:
			import exception
			exception.Abort("GiftBox.LoadWindow.LoadObject: {}".format(e))

		self.giftBoxIcon = self.GetChild("GiftBox_Icon")
		self.giftBoxToolTip = self.GetChild("GiftBox_ToolTip")

	def Destroy(self):
		self.giftBoxIcon = None
		self.giftBoxToolTip = None

		if self.tooltipGift:
			self.tooltipGift.Hide()
			self.tooltipGift = None

		self.Hide()

class EnergyBar(ui.ScriptWindow):
	class TextToolTip(ui.Window):
		def __init__(self):
			ui.Window.__init__(self, "TOP_MOST")
			self.SetWindowName("EnergyBar")
			textLine = ui.TextLine()
			textLine.SetParent(self)
			textLine.SetHorizontalAlignCenter()
			textLine.SetOutline()
			textLine.Show()
			self.textLine = textLine

		def __del__(self):
			ui.Window.__del__(self)

		def SetText(self, text):
			self.textLine.SetText(text)

		def OnRender(self):
			(mouseX, mouseY) = wndMgr.GetMousePosition()
			self.textLine.SetPosition(mouseX, mouseY - 15)

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.tooltipEnergy = self.TextToolTip()
		self.tooltipEnergy.Show()
		
	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, uiScriptLocale.LOCALE_UISCRIPT_PATH + "energybar.py")
		except:
			import exception
			exception.Abort("EnergyBar.LoadWindow.LoadObject")

		self.energyEmpty = self.GetChild("EnergyGauge_Empty")
		self.energyHungry = self.GetChild("EnergyGauge_Hungry")
		self.energyFull = self.GetChild("EnergyGauge_Full")
		self.energyGaugeBoard = self.GetChild("EnergyGauge_Board")
		self.energyGaugeToolTip = self.GetChild("EnergyGauge_ToolTip")

	def Destroy(self):
		self.energyEmpty = None
		self.energyHungry = None
		self.energyFull = None
		self.energyGaugeBoard = 0
		self.energyGaugeToolTip = 0
		self.tooltipEnergy = 0

	def RefreshStatus(self):
		pointEnergy = player.GetStatus (player.ENERGY)
		leftTimeEnergy = player.GetStatus (player.ENERGY_END_TIME) - app.GetGlobalTimeStamp()
		self.SetEnergy (pointEnergy, leftTimeEnergy, 7200)

	def SetEnergy (self, point, leftTime, maxTime):
		leftTime = max (leftTime, 0)
		maxTime = max (maxTime, 0)

		self.energyEmpty.Hide()
		self.energyHungry.Hide()
		self.energyFull.Hide()

		if leftTime == 0:
			self.energyEmpty.Show()
		elif ((leftTime * 100) / maxTime) < 15:
			self.energyHungry.Show()
		else:
			self.energyFull.Show()

		self.tooltipEnergy.SetText("%s" % (localeInfo.TOOLTIP_ENERGY(point)))

	def OnUpdate(self):
		if True == self.energyGaugeToolTip.IsIn():
			self.RefreshStatus()
			self.tooltipEnergy.Show()
		else:
			self.tooltipEnergy.Hide()

class ExpandedTaskBar(ui.ScriptWindow):
	BUTTON_DRAGON_SOUL = 0

	def __init__(self):
		ui.Window.__init__(self)
		self.SetWindowName("ExpandedTaskBar")

	def LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, uiScriptLocale.LOCALE_UISCRIPT_PATH + "expandedtaskbar.py")
		except:
			import exception
			exception.Abort("ExpandedTaskBar.LoadWindow.LoadObject")

		self.expandedTaskBarBoard = self.GetChild("ExpanedTaskBar_Board")

		self.toggleButtonDict = {}
		self.toggleButtonDict[ExpandedTaskBar.BUTTON_DRAGON_SOUL] = self.GetChild("DragonSoulButton")
		self.toggleButtonDict[ExpandedTaskBar.BUTTON_DRAGON_SOUL].SetParent(self)

	def SetTop(self):
		super(ExpandedTaskBar, self).SetTop()	
		for button in self.toggleButtonDict.values():
			button.SetTop()

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()

	def SetToolTipText(self, eButton, text):
		self.toggleButtonDict[eButton].SetToolTipText(text)

	def SetToggleButtonEvent(self, eButton, kEventFunc):
		self.toggleButtonDict[eButton].SetEvent(kEventFunc)

	def OnPressEscapeKey(self):
		self.Close()
		return True

class TaskBar(ui.ScriptWindow):
	BUTTON_CHARACTER = 0
	BUTTON_INVENTORY = 1
	BUTTON_MESSENGER = 2
	BUTTON_SYSTEM = 3
	BUTTON_CHAT = 4
	BUTTON_EXPAND = 4
	IS_EXPANDED = True
	MOUSE_BUTTON_LEFT = 0
	MOUSE_BUTTON_RIGHT = 1
	NONE = 255
	EVENT_MOVE = 0
	EVENT_ATTACK = 1
	EVENT_MOVE_AND_ATTACK = 2
	EVENT_CAMERA = 3
	EVENT_SKILL = 4
	EVENT_AUTO = 5
	GAUGE_WIDTH = 95
	GAUGE_HEIGHT = 13

	QUICKPAGE_NUMBER_FILENAME = [
		"d:/ymir work/ui/game/taskbar/1.sub",
		"d:/ymir work/ui/game/taskbar/2.sub",
		"d:/ymir work/ui/game/taskbar/3.sub",
		"d:/ymir work/ui/game/taskbar/4.sub",
	]

	def ShowGift(self):
		self.wndGiftBox.Show()

	def HideGift(self):
		self.wndGiftBox.Hide()

	class TextToolTip(ui.Window):
		def __init__(self):
			super(TaskBar.TextToolTip, self).__init__("TOP_MOST")

			self.textLine = ui.TextLine()
			self.textLine.SetParent(self)
			self.textLine.SetHorizontalAlignCenter()
			self.textLine.SetOutline()
			self.textLine.Show()

		def __del__(self):
			if self.textLine:
				self.textLine.Hide()
				self.textLine = None
			super(TaskBar.TextToolTip, self).__del__()

		def SetText(self, text):
			self.textLine.SetText(text)

		def OnRender(self):
			(mouseX, mouseY) = wndMgr.GetMousePosition()
			self.textLine.SetPosition(mouseX, mouseY - 15)

	class SkillButton(ui.SlotWindow):
		def __init__(self):
			super(TaskBar.SkillButton, self).__init__()
			self.event = None
			self.arg = None
			self.slotIndex = 0
			self.skillIndex = 0

			wndMgr.SetSlotBaseImage(self.hWnd, "d:/ymir work/ui/public/slot_base.sub", 1.0, 1.0, 1.0, 1.0)
			wndMgr.AppendSlot(self.hWnd, self.slotIndex, 0, 0, 32, 32)
			self.SetCoverButton(self.slotIndex,	"d:/ymir work/ui/public/slot_cover_button_01.sub",\
											"d:/ymir work/ui/public/slot_cover_button_02.sub",\
											"d:/ymir work/ui/public/slot_cover_button_03.sub",\
											"d:/ymir work/ui/public/slot_cover_button_04.sub", True, False)
			self.SetSize(32, 32)

		def __del__(self):
			if self.tooltipSkill:
				self.tooltipSkill.HideToolTip()
				self.tooltipSkill = None

			super(TaskBar.SkillButton, self).__del__()

		def Destroy(self):
			if self.tooltipSkill:
				self.tooltipSkill.HideToolTip()
				self.tooltipSkill = None

			self.event = None
			self.arg = None

		def RefreshSkill(self):
			if self.slotIndex != 0:
				self.SetSkill(self.slotIndex)

		def SetSkillToolTip(self, tooltip):
			self.tooltipSkill = tooltip

		def SetSkill(self, skillSlotNumber):
			skillIndex = player.GetSkillIndex(skillSlotNumber)
			skillGrade = player.GetSkillGrade(skillSlotNumber)
			skillLevel = player.GetSkillLevel(skillSlotNumber)
			skillType = skill.GetSkillType(skillIndex)

			self.skillIndex = skillIndex
			if self.skillIndex == 0:
				self.ClearSlot(self.slotIndex)
				return

			self.slotIndex = skillSlotNumber
			self.SetSkillSlotNew(self.slotIndex, skillIndex, skillGrade, skillLevel)
			self.SetSlotCountNew(self.slotIndex, skillGrade, skillLevel)

			if player.IsSkillCoolTime(skillSlotNumber):
				coolTime, elapsedTime = player.GetSkillCoolTime(skillSlotNumber)
				self.SetSlotCoolTime(self.slotIndex, coolTime, elapsedTime)

			if player.IsSkillActive(skillSlotNumber):
				self.ActivateSlot(self.slotIndex)

		def SetSkillEvent(self, event, arg = 0):
			self.event = event
			self.arg = arg

		def GetSkillIndex(self):
			return self.skillIndex

		def GetSlotIndex(self):
			return self.slotIndex

		def Activate(self, coolTime):
			self.SetSlotCoolTime(self.slotIndex, coolTime)
			if skill.IsToggleSkill(self.skillIndex):
				self.ActivateSlot(self.slotIndex)

		def Deactivate(self):
			if skill.IsToggleSkill(self.skillIndex):
				self.DeactivateSlot(self.slotIndex)

		def OnOverInItem(self, dummy):
			if self.tooltipSkill:
				self.tooltipSkill.SetSkill(self.skillIndex)

		def OnOverOutItem(self):
			if self.tooltipSkill:
				self.tooltipSkill.HideToolTip()

		def OnSelectItemSlot(self, dummy):
			if self.event:
				if self.arg:
					self.event(self.arg)
				else:
					self.event()

	def __init__(self):
		super(TaskBar, self).__init__("TOP_MOST")

		self.quickPageNumImageBox = None
		self.tooltipItem = None
		self.tooltipSkill = None
		self.mouseModeButtonList = [ui.ScriptWindow("TOP_MOST"), ui.ScriptWindow("TOP_MOST")]

		self.tooltipHP = self.CreateToolTip()
		self.tooltipSP = self.CreateToolTip()
		self.tooltipST = self.CreateToolTip()
		self.tooltipEXP = self.CreateToolTip()

		self.skillCategoryNameList = ["ACTIVE_1", "ACTIVE_2", "ACTIVE_3"]
		self.skillPageStartSlotIndexDict = {"ACTIVE_1" : 1, "ACTIVE_2" : 21, "ACTIVE_3" : 41}

		self.selectSkillButtonList = []
		self.lastUpdateQuickSlot = 0
		self.SetWindowName("TaskBar")

		self.mouseSettings = MouseSettings()

	def CreateToolTip(self):
		tooltip = self.TextToolTip()
		tooltip.Show()
		return tooltip

	def __del__(self):
		super(TaskBar, self).__del__()

	def ClearResources(self):
		for attr in ["mouseModeButtonList", "quickPageNumImageBox", "quickslot", "toggleButtonDict",
					 "expGauge", "hpGauge", "mpGauge", "stGauge", "hpRecoveryGaugeBar",
					 "spRecoveryGaugeBar", "tooltipHP", "tooltipSP", "tooltipST",
					 "tooltipEXP", "mouseImage", "curSkillButton", "selectSkillButtonList"]:
			setattr(self, attr, None)
			
	def Destroy(self):
		self.mouseSettings.SaveMouseButtonSettings()
		self.ClearDictionary()

		if self.mouseModeButtonList:
			for button in self.mouseModeButtonList:
				button.ClearDictionary()

		self.ClearResources()

		if self.quickPageNumImageBox:
			self.quickPageNumImageBox.ClearDictionary()
			self.quickPageNumImageBox = None

		if self.wndGiftBox:
			self.wndGiftBox.Destroy()
			self.wndGiftBox = None

	def LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/taskbar.py")
			pyScrLoader.LoadScriptFile(self.mouseModeButtonList[self.MOUSE_BUTTON_LEFT], "uiscript/mousebuttonwindow.py")
			pyScrLoader.LoadScriptFile(self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT], "uiscript/rightmousebuttonwindow.py")
		except:
			import exception
			exception.Abort("TaskBar.LoadWindow.LoadObject")

		self.quickslot = [self.GetChild("quick_slot_1"), self.GetChild("quick_slot_2")]
		for slot in self.quickslot:
			self.InitializeSlot(slot)

		toggleButtonDict = {
			TaskBar.BUTTON_CHARACTER: self.GetChild("CharacterButton"),
			TaskBar.BUTTON_INVENTORY: self.GetChild("InventoryButton"),
			TaskBar.BUTTON_MESSENGER: self.GetChild("MessengerButton"),
			TaskBar.BUTTON_SYSTEM: self.GetChild("SystemButton"),
		}

		self.InitializeToggleButtons(toggleButtonDict)
		
		if localeInfo.IsARABIC():
			self.AdjustSystemButtonTooltip(toggleButtonDict)

		self.expGauge = [self.GetChild("EXPGauge_0%d" % (i + 1)) for i in range(4)]
		for exp in self.expGauge:
			exp.SetSize(0, 0)		
	  
		self.quickPageNumImageBox=self.GetChild("QuickPageNumber")
		self.InitializeQuickPageButtons()
		self.InitializeGauges()
		self.InitializeMouseButtons()
		self.InitializeMouseButtons()
		self.InitializeMouseEvents()
		self.InitializeSkillButton()
		self.InitializeMouseModeButtonPositions()
		self.InitializeMouseImage()

		wndGiftBox = GiftBox()
		wndGiftBox.LoadWindow()
		self.wndGiftBox = wndGiftBox

		self.__LoadMouseSettings()
		self.RefreshStatus()
		self.RefreshQuickSlot()
		
	def InitializeSlot(self, slot):
		slot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		slot.SetSelectEmptySlotEvent(ui.__mem_func__(self.SelectEmptyQuickSlot))
		slot.SetSelectItemSlotEvent(ui.__mem_func__(self.SelectItemQuickSlot))
		slot.SetUnselectItemSlotEvent(ui.__mem_func__(self.UnselectItemQuickSlot))
		slot.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
		slot.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem))	
		
	def InitializeToggleButtons(self, toggleButtonDict):
		try:
			toggleButtonDict[TaskBar.BUTTON_CHAT] = self.GetChild("ChatButton")
		except:
			toggleButtonDict[TaskBar.BUTTON_EXPAND] = self.GetChild("ExpandButton")
			TaskBar.IS_EXPANDED = True

		self.toggleButtonDict = toggleButtonDict

	def AdjustSystemButtonTooltip(self, toggleButtonDict):
		systemButton = toggleButtonDict[TaskBar.BUTTON_SYSTEM]
		if systemButton.ToolTipText:
			tx, ty = systemButton.ToolTipText.GetLocalPosition()
			tw = systemButton.ToolTipText.GetWidth()
			systemButton.ToolTipText.SetPosition(-tw / 2, ty)	

	def InitializeQuickPageButtons(self):
		self.GetChild("QuickPageUpButton").SetEvent(ui.__mem_func__(self.__OnClickQuickPageUpButton))
		self.GetChild("QuickPageDownButton").SetEvent(ui.__mem_func__(self.__OnClickQuickPageDownButton))			
			
	def InitializeGauges(self):
		self.hpGauge = self.GetChild("HPGauge")
		self.mpGauge = self.GetChild("SPGauge")
		self.stGauge = self.GetChild("STGauge")
		self.hpRecoveryGaugeBar = self.GetChild("HPRecoveryGaugeBar")
		self.spRecoveryGaugeBar = self.GetChild("SPRecoveryGaugeBar")
		self.hpGaugeBoard = self.GetChild("HPGauge_Board")
		self.mpGaugeBoard = self.GetChild("SPGauge_Board")
		self.stGaugeBoard = self.GetChild("STGauge_Board")
		self.expGaugeBoard = self.GetChild("EXP_Gauge_Board")

	def InitializeMouseButtons(self):
		self.mouseLeftButtonModeButton = self.GetChild("LeftMouseButton")
		self.mouseRightButtonModeButton = self.GetChild("RightMouseButton")

	def InitializeMouseEvents(self):
		self.mouseLeftButtonModeButton.SetEvent(ui.__mem_func__(self.ToggleLeftMouseButtonModeWindow))        
		self.mouseRightButtonModeButton.SetEvent(ui.__mem_func__(self.ToggleRightMouseButtonModeWindow))
		self.curMouseModeButton = [self.mouseLeftButtonModeButton, self.mouseRightButtonModeButton]

	def InitializeSkillButton(self):
		(xLocalRight, yLocalRight) = self.mouseRightButtonModeButton.GetLocalPosition()
		self.curSkillButton = self.SkillButton()
		self.curSkillButton.SetParent(self)
		self.curSkillButton.SetPosition(xLocalRight, 3)
		self.curSkillButton.SetSkillEvent(ui.__mem_func__(self.ToggleRightMouseButtonModeWindow))
		self.curSkillButton.Hide()

	def InitializeMouseModeButtonPositions(self):
		(xLeft, yLeft) = self.mouseLeftButtonModeButton.GetGlobalPosition()
		(xRight, yRight) = self.mouseRightButtonModeButton.GetGlobalPosition()
		
		leftModeButtonList = self.mouseModeButtonList[self.MOUSE_BUTTON_LEFT]
		leftModeButtonList.SetPosition(xLeft, yLeft - leftModeButtonList.GetHeight() - 5)
		
		rightModeButtonList = self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT]
		rightModeButtonList.SetPosition(xRight - rightModeButtonList.GetWidth() + 32, yRight - rightModeButtonList.GetHeight() - 5)
		
		rightModeButtonList.GetChild("button_skill").SetEvent(lambda adir=self.MOUSE_BUTTON_RIGHT, aevent=self.EVENT_SKILL: self.SelectMouseButtonEvent(adir, aevent))
		rightModeButtonList.GetChild("button_skill").Hide()

		dir = self.MOUSE_BUTTON_LEFT
		wnd = self.mouseModeButtonList[dir]
		wnd.GetChild("button_move_and_attack").SetEvent(lambda adir=dir, aevent=self.EVENT_MOVE_AND_ATTACK: self.SelectMouseButtonEvent(adir, aevent))
		wnd.GetChild("button_auto_attack").SetEvent(lambda adir=dir, aevent=self.EVENT_AUTO: self.SelectMouseButtonEvent(adir, aevent))
		wnd.GetChild("button_camera").SetEvent(lambda adir=dir, aevent=self.EVENT_CAMERA: self.SelectMouseButtonEvent(adir, aevent))

		dir = self.MOUSE_BUTTON_RIGHT
		wnd = self.mouseModeButtonList[dir]
		wnd.GetChild("button_move_and_attack").SetEvent(lambda adir=dir, aevent=self.EVENT_MOVE_AND_ATTACK: self.SelectMouseButtonEvent(adir, aevent))
		wnd.GetChild("button_camera").SetEvent(lambda adir=dir, aevent=self.EVENT_CAMERA: self.SelectMouseButtonEvent(adir, aevent))

	def InitializeMouseImage(self):
		(xRight, _) = self.mouseRightButtonModeButton.GetGlobalPosition()
		mouseImage = ui.ImageBox("TOP_MOST")
		mouseImage.AddFlag("float")
		mouseImage.LoadImage("d:/ymir work/ui/game/taskbar/mouse_button_camera_01.sub")
		mouseImage.SetPosition(xRight, wndMgr.GetScreenHeight() - 34)
		mouseImage.Hide()
		self.mouseImage = mouseImage		

	def __LoadMouseSettings(self):
		try:
			self.mouseSettings.LoadMouseButtonSettings()
			(mouseLeftButtonEvent, mouseRightButtonEvent) = self.mouseSettings.GetMouseButtonSettings()
			if not self.__IsInSafeMouseButtonSettingRange(mouseLeftButtonEvent) or not self.__IsInSafeMouseButtonSettingRange(mouseRightButtonEvent):
				sys_err("INVALID_MOUSE_BUTTON_SETTINGS")
		except:
			self.mouseSettings.InitMouseButtonSettings(self.EVENT_MOVE_AND_ATTACK, self.EVENT_CAMERA)
			(mouseLeftButtonEvent, mouseRightButtonEvent) = self.mouseSettings.GetMouseButtonSettings()

		try:
			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_LEFT, mouseLeftButtonEvent)
			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_RIGHT, mouseRightButtonEvent)
		except:
			self.mouseSettings.InitMouseButtonSettings(self.EVENT_MOVE_AND_ATTACK, self.EVENT_CAMERA)
			(mouseLeftButtonEvent, mouseRightButtonEvent) = self.mouseSettings.GetMouseButtonSettings()

			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_LEFT, mouseLeftButtonEvent)
			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_RIGHT, mouseRightButtonEvent)

	def __IsInSafeMouseButtonSettingRange(self, arg):
		return arg >= self.EVENT_MOVE and arg <= self.EVENT_AUTO

	def __OnClickQuickPageUpButton(self):
		player.SetQuickPage(player.GetQuickPage()-1)

	def __OnClickQuickPageDownButton(self):
		player.SetQuickPage(player.GetQuickPage()+1)

	def SetToggleButtonEvent(self, eButton, kEventFunc):
		self.toggleButtonDict[eButton].SetEvent(kEventFunc)

	def SetItemToolTip(self, tooltipItem):
		self.tooltipItem = tooltipItem

	def SetSkillToolTip(self, tooltipSkill):
		self.tooltipSkill = tooltipSkill
		self.curSkillButton.SetSkillToolTip(self.tooltipSkill)

	def ShowMouseImage(self):
		self.mouseImage.SetTop()
		self.mouseImage.Show()

	def HideMouseImage(self):
		player.SetQuickCameraMode(FALSE)
		self.mouseImage.Hide()

	def RefreshStatus(self):
		curHP = player.GetStatus(player.HP)
		maxHP = player.GetStatus(player.MAX_HP)
		curSP = player.GetStatus(player.SP)
		maxSP = player.GetStatus(player.MAX_SP)
		curEXP = MouseSettings.unsigned32(player.GetStatus(player.EXP))
		nextEXP = MouseSettings.unsigned32(player.GetStatus(player.NEXT_EXP))
		recoveryHP = player.GetStatus(player.HP_RECOVERY)
		recoverySP = player.GetStatus(player.SP_RECOVERY)

		self.RefreshStamina()

		self.SetHP(curHP, recoveryHP, maxHP)
		self.SetSP(curSP, recoverySP, maxSP)
		self.SetExperience(curEXP, nextEXP)

	def RefreshStamina(self):
		curST = player.GetStatus(player.STAMINA)
		maxST = player.GetStatus(player.MAX_STAMINA)
		self.SetST(curST, maxST)

	def RefreshSkill(self):
		self.curSkillButton.RefreshSkill()
		for button in self.selectSkillButtonList:
			button.RefreshSkill()

	def SetHP(self, curPoint, recoveryPoint, maxPoint):
		curPoint = min(curPoint, maxPoint)
		if maxPoint > 0:
			self.hpGauge.SetPercentage(curPoint, maxPoint)
			self.tooltipHP.SetText("%s : %d / %d" % (localeInfo.TASKBAR_HP, curPoint, maxPoint))

			if recoveryPoint == 0:
				self.hpRecoveryGaugeBar.Hide()
			else:
				destPoint = min(maxPoint, curPoint + recoveryPoint)
				newWidth = int(self.GAUGE_WIDTH * (destPoint / float(maxPoint)))
				self.hpRecoveryGaugeBar.SetSize(newWidth, self.GAUGE_HEIGHT)
				self.hpRecoveryGaugeBar.Show()

	def SetSP(self, curPoint, recoveryPoint, maxPoint):
		curPoint = min(curPoint, maxPoint)
		if maxPoint > 0:
			self.mpGauge.SetPercentage(curPoint, maxPoint)
			self.tooltipSP.SetText("%s : %d / %d" % (localeInfo.TASKBAR_SP, curPoint, maxPoint))

			if recoveryPoint == 0:
				self.spRecoveryGaugeBar.Hide()
			else:
				destPoint = min(maxPoint, curPoint + recoveryPoint)
				newWidth = int(self.GAUGE_WIDTH * (destPoint) / float(maxPoint))
				self.spRecoveryGaugeBar.SetSize(newWidth, self.GAUGE_HEIGHT)
				self.spRecoveryGaugeBar.Show()

	def SetST(self, curPoint, maxPoint):
		curPoint = min(curPoint, maxPoint)
		if maxPoint > 0:
			self.stGauge.SetPercentage(curPoint, maxPoint)
			self.tooltipST.SetText("%s : %d / %d" % (localeInfo.TASKBAR_ST, curPoint, maxPoint))
			
	def SetExperience(self, curPoint, maxPoint):
		curPoint = min(max(curPoint, 0), maxPoint)
		maxPoint = max(maxPoint, 0)

		quarterPoint = maxPoint / 4
		fullCount = min(4, curPoint / quarterPoint) if quarterPoint > 0 else 0

		for i in xrange(4):
			self.expGauge[i].Hide()

		for i in xrange(fullCount):
			self.expGauge[i].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
			self.expGauge[i].Show()

		if quarterPoint > 0 and fullCount < 4:
			percentage = float(curPoint % quarterPoint) / quarterPoint - 1.0
			self.expGauge[fullCount].SetRenderingRect(0.0, percentage, 0.0, 0.0)
			self.expGauge[fullCount].Show()

		self.tooltipEXP.SetText("%s : %.2f%%" % (localeInfo.TASKBAR_EXP, float(curPoint) / max(1, float(maxPoint)) * 100))			

	def RefreshQuickSlot(self):
		pageNum = player.GetQuickPage()

		try:
			self.quickPageNumImageBox.LoadImage(TaskBar.QUICKPAGE_NUMBER_FILENAME[pageNum])
		except IndexError:
			pass

		startNumber = 0
		for slot in self.quickslot:
			for i in xrange(4):
				slotNumber = i + startNumber
				slotType, position = player.GetLocalQuickSlot(slotNumber)

				if slotType == player.SLOT_TYPE_NONE:
					slot.ClearSlot(slotNumber)
				elif slotType == player.SLOT_TYPE_INVENTORY:
					self.SetInventorySlot(slot, slotNumber, position)
				elif slotType == player.SLOT_TYPE_SKILL:
					self.SetSkillSlot(slot, slotNumber, position)
				elif slotType == player.SLOT_TYPE_EMOTION:
					self.SetEmotionSlot(slot, slotNumber, position)

			slot.RefreshSlot()
			startNumber += 4
			
	def SetInventorySlot(self, slot, slotNumber, position):
		itemIndex = player.GetItemIndex(position)
		itemCount = player.GetItemCount(position)
		if itemCount <= 1:
			itemCount = 0

		if constInfo.IS_AUTO_POTION(itemIndex):
			metinSocket = [player.GetItemMetinSocket(position, j) for j in xrange(player.METIN_SOCKET_MAX_NUM)]
			if int(metinSocket[0]) != 0:
				slot.ActivateSlot(slotNumber)
			else:
				slot.DeactivateSlot(slotNumber)
		elif constInfo.IS_PET_SEAL(itemIndex):
			metinSocket = [player.GetItemMetinSocket(position, j) for j in xrange(player.METIN_SOCKET_MAX_NUM)]
			if int(metinSocket[2]) != 0:
				slot.ActivateSlot(slotNumber)
			else:
				slot.DeactivateSlot(slotNumber)
		slot.SetItemSlot(slotNumber, itemIndex, itemCount)

	def SetSkillSlot(self, slot, slotNumber, position):
		skillIndex = player.GetSkillIndex(position)
		if skillIndex == 0:
			slot.ClearSlot(slotNumber)
			return

		skillType = skill.GetSkillType(skillIndex)
		if skillType == skill.SKILL_TYPE_GUILD:
			import guild
			skillGrade = 0
			skillLevel = guild.GetSkillLevel(position)
		else:
			skillGrade = player.GetSkillGrade(position)
			skillLevel = player.GetSkillLevel(position)

		slot.SetSkillSlotNew(slotNumber, skillIndex, skillGrade, skillLevel)
		slot.SetSlotCountNew(slotNumber, skillGrade, skillLevel)
		slot.SetCoverButton(slotNumber)

		if player.IsSkillCoolTime(position):
			coolTime, elapsedTime = player.GetSkillCoolTime(position)
			slot.SetSlotCoolTime(slotNumber, coolTime, elapsedTime)

		if player.IsSkillActive(position):
			slot.ActivateSlot(slotNumber)

	def SetEmotionSlot(self, slot, slotNumber, position):
		emotionIndex = position
		slot.SetEmotionSlot(slotNumber, emotionIndex)
		slot.SetCoverButton(slotNumber)
		slot.SetSlotCount(slotNumber, 0)			

	def canAddQuickSlot(self, Type, slotNumber):
		if Type == player.SLOT_TYPE_INVENTORY:
			itemIndex = player.GetItemIndex(slotNumber)
			return item.CanAddToQuickSlotItem(itemIndex)
		return True

	def AddQuickSlot(self, localSlotIndex):
		attachedSlotType = mouseModule.mouseController.GetAttachedType()
		attachedSlotNumber = mouseModule.mouseController.GetAttachedSlotNumber()
		attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

		if attachedSlotType == player.SLOT_TYPE_QUICK_SLOT:
			player.RequestMoveGlobalQuickSlotToLocalQuickSlot(attachedSlotNumber, localSlotIndex)
		elif attachedSlotType == player.SLOT_TYPE_EMOTION:
			player.RequestAddLocalQuickSlot(localSlotIndex, attachedSlotType, attachedItemIndex)
		elif self.canAddQuickSlot(attachedSlotType, attachedSlotNumber):
			player.RequestAddLocalQuickSlot(localSlotIndex, attachedSlotType, attachedSlotNumber)

		mouseModule.mouseController.DeattachObject()
		self.RefreshQuickSlot()

	def SelectEmptyQuickSlot(self, slotIndex):
		if mouseModule.mouseController.isAttached():
			self.AddQuickSlot(slotIndex)

	def SelectItemQuickSlot(self, localQuickSlotIndex):
		if mouseModule.mouseController.isAttached():
			self.AddQuickSlot(localQuickSlotIndex)
		else:
			globalQuickSlotIndex = player.LocalQuickSlotIndexToGlobalQuickSlotIndex(localQuickSlotIndex)
			mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_QUICK_SLOT, globalQuickSlotIndex, globalQuickSlotIndex)

	def UnselectItemQuickSlot(self, localSlotIndex):
		if not mouseModule.mouseController.isAttached():
			player.RequestUseLocalQuickSlot(localSlotIndex)
		else:
			mouseModule.mouseController.DeattachObject()

	def OnUseSkill(self, usedSlotIndex, coolTime):
		QUICK_SLOT_SLOT_COUNT = 4
		slotIndex = 0

		if usedSlotIndex == self.curSkillButton.GetSlotIndex():
			self.curSkillButton.Activate(coolTime)

		for slotWindow in self.quickslot:
			for i in xrange(QUICK_SLOT_SLOT_COUNT):
				slotType, position = player.GetLocalQuickSlot(slotIndex)
				if slotType == player.SLOT_TYPE_SKILL and usedSlotIndex == position:
					slotWindow.SetSlotCoolTime(slotIndex, coolTime)
					return
				slotIndex += 1


	def OnActivateSkill(self, usedSlotIndex):
		self.UpdateSkillSlot(usedSlotIndex, self.curSkillButton.Deactivate, 'ActivateSlot')

	def OnDeactivateSkill(self, usedSlotIndex):
		self.UpdateSkillSlot(usedSlotIndex, self.curSkillButton.Deactivate, 'DeactivateSlot')

	def UpdateSkillSlot(self, usedSlotIndex, deactivateCurSkillFunc, quickSlotFunc):
		slotIndex = 0

		if usedSlotIndex == self.curSkillButton.GetSlotIndex():
			deactivateCurSkillFunc()

		for slotWindow in self.quickslot:
			for i in xrange(4):
				slotType, position = player.GetLocalQuickSlot(slotIndex)
				if slotType == player.SLOT_TYPE_SKILL and usedSlotIndex == position:
					getattr(slotWindow, quickSlotFunc)(slotIndex)
					return
				slotIndex += 1

	def OverInItem(self, slotNumber):
		if mouseModule.mouseController.isAttached():
			return

		slotType, position = player.GetLocalQuickSlot(slotNumber)

		if slotType == player.SLOT_TYPE_INVENTORY:
			self.tooltipItem.SetInventoryItem(position)
			self.tooltipSkill.HideToolTip()
		elif slotType == player.SLOT_TYPE_SKILL:
			skillIndex = player.GetSkillIndex(position)
			skillType = skill.GetSkillType(skillIndex)

			if skillType == skill.SKILL_TYPE_GUILD:
				import guild
				skillGrade = 0
				skillLevel = guild.GetSkillLevel(position)
			else:
				skillGrade = player.GetSkillGrade(position)
				skillLevel = player.GetSkillLevel(position)

			self.tooltipSkill.SetSkillNew(position, skillIndex, skillGrade, skillLevel)
			self.tooltipItem.HideToolTip()

	def OverOutItem(self):
		if self.tooltipItem:
			self.tooltipItem.HideToolTip()
		if self.tooltipSkill:
			self.tooltipSkill.HideToolTip()

	def OnUpdate(self):
		currentTime = app.GetGlobalTime()
		if currentTime - self.lastUpdateQuickSlot > 500:
			self.lastUpdateQuickSlot = currentTime
			self.RefreshQuickSlot()

		self.tooltipHP.Show() if self.hpGaugeBoard.IsIn() else self.tooltipHP.Hide()
		self.tooltipSP.Show() if self.mpGaugeBoard.IsIn() else self.tooltipSP.Hide()
		self.tooltipST.Show() if self.stGaugeBoard.IsIn() else self.tooltipST.Hide()
		self.tooltipEXP.Show() if self.expGaugeBoard.IsIn() else self.tooltipEXP.Hide()
						
	def ToggleLeftMouseButtonModeWindow(self):
		self.ToggleMouseButtonModeWindow(self.MOUSE_BUTTON_LEFT)

	def ToggleRightMouseButtonModeWindow(self):
		self.ToggleMouseButtonModeWindow(self.MOUSE_BUTTON_RIGHT, self.OpenSelectSkill, self.CloseSelectSkill)

	def ToggleMouseButtonModeWindow(self, buttonIndex, onShow=None, onHide=None):
		wndMouseButtonMode = self.mouseModeButtonList[buttonIndex]

		if wndMouseButtonMode.IsShow():
			wndMouseButtonMode.Hide()
			if onHide:
				onHide()
		else:
			wndMouseButtonMode.Show()
			if onShow:
				onShow()

	def OpenSelectSkill(self):
		PAGE_SLOT_COUNT = 6

		(xSkillButton, y) = self.curSkillButton.GetGlobalPosition()
		y -= 70

		getSkillIndex = player.GetSkillIndex
		getSkillLevel = player.GetSkillLevel
		
		for key in self.skillCategoryNameList:
			appendCount = 0
			startNumber = self.skillPageStartSlotIndexDict[key]
			x = xSkillButton

			for i in xrange(PAGE_SLOT_COUNT):
				skillIndex = getSkillIndex(startNumber + i)
				skillLevel = getSkillLevel(startNumber + i)

				if skillIndex == 0 or skillLevel == 0 or skill.IsStandingSkill(skillIndex):
					continue

				skillButton = self.SkillButton()
				skillButton.SetSkill(startNumber + i)
				skillButton.SetPosition(x, y)
				skillButton.SetSkillEvent(ui.__mem_func__(self.CloseSelectSkill), startNumber + i + 1)
				skillButton.SetSkillToolTip(self.tooltipSkill)
				skillButton.SetTop()
				skillButton.Show()
				self.selectSkillButtonList.append(skillButton)

				appendCount += 1
				x -= 32

			if appendCount > 0:
				y -= 32

	def CloseSelectSkill(self, slotIndex=-1):
		self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT].Hide()
		for button in self.selectSkillButtonList:
			button.Destroy()

		self.selectSkillButtonList = []

		if slotIndex != -1:
			self.curSkillButton.Show()
			self.curMouseModeButton[self.MOUSE_BUTTON_RIGHT].Hide()
			player.SetMouseFunc(player.MBT_RIGHT, player.MBF_SKILL)
			player.ChangeCurrentSkillNumberOnly(slotIndex - 1)
		else:
			self.curSkillButton.Hide()
			self.curMouseModeButton[self.MOUSE_BUTTON_RIGHT].Show()

	def SelectMouseButtonEvent(self, dir, event):
		self.mouseSettings.SetMouseButtonSetting(dir, event)
		self.CloseSelectSkill()
		self.mouseModeButtonList[dir].Hide()

		btn = None
		func = self.NONE
		tooltip_text = ""

		if dir == self.MOUSE_BUTTON_LEFT:
			type = player.MBT_LEFT
		else:
			type = player.MBT_RIGHT

		event_mapping = {
			self.EVENT_MOVE: ("button_move", player.MBF_MOVE, localeInfo.TASKBAR_MOVE),
			self.EVENT_ATTACK: ("button_attack", player.MBF_ATTACK, localeInfo.TASKBAR_ATTACK),
			self.EVENT_AUTO: ("button_auto_attack", player.MBF_AUTO, localeInfo.TASKBAR_AUTO),
			self.EVENT_MOVE_AND_ATTACK: ("button_move_and_attack", player.MBF_SMART, localeInfo.TASKBAR_ATTACK),
			self.EVENT_CAMERA: ("button_camera", player.MBF_CAMERA, localeInfo.TASKBAR_CAMERA),
			self.EVENT_SKILL: ("button_skill", player.MBF_SKILL, localeInfo.TASKBAR_SKILL),
		}

		if event in event_mapping:
			btn_name, func, tooltip_text = event_mapping[event]
			btn = self.mouseModeButtonList[dir].GetChild(btn_name)

		if btn:
			self.curMouseModeButton[dir].SetToolTipText(tooltip_text, 0, -18)
			self.curMouseModeButton[dir].SetUpVisual(btn.GetUpVisualFileName())
			self.curMouseModeButton[dir].SetOverVisual(btn.GetOverVisualFileName())
			self.curMouseModeButton[dir].SetDownVisual(btn.GetDownVisualFileName())
			self.curMouseModeButton[dir].Show()

		player.SetMouseFunc(type, func)

	def OnChangeCurrentSkill(self, skillSlotNumber):
		self.curSkillButton.SetSkill(skillSlotNumber)
		self.curSkillButton.Show()
		self.curMouseModeButton[self.MOUSE_BUTTON_RIGHT].Hide()
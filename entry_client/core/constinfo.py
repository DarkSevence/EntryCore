import item
import systemSetting

# option
IN_GAME_SHOP_ENABLE = 1
CONSOLE_ENABLE = 0

PVPMODE_ENABLE = 1
PVPMODE_TEST_ENABLE = 0
PVPMODE_ACCELKEY_ENABLE = 1
PVPMODE_ACCELKEY_DELAY = 0.5
PVPMODE_PROTECTED_LEVEL = 30
ACCOUNT_NAME = "NoName"

CAMERA_MAX_DISTANCE_SHORT = 2500.0
CAMERA_MAX_DISTANCE_LONG = 8500.0
CAMERA_MAX_DISTANCE_LIST=[CAMERA_MAX_DISTANCE_SHORT, CAMERA_MAX_DISTANCE_LONG]
CAMERA_MAX_DISTANCE = CAMERA_MAX_DISTANCE_LIST[systemSetting.GetCameraMode()]

CHRNAME_COLOR_INDEX = 0

ENVIRONMENT_NIGHT="d:/ymir work/environment/moonlight.msenv"

PRIVATE_SHOP_TOTAL_GOLD = 0

# constant
HIGH_PRICE = 500000
MIDDLE_PRICE = 50000
ERROR_METIN_STONE = 28960
SUB2_LOADING_ENABLE = 1
EXPANDED_COMBO_ENABLE = 1
CONVERT_EMPIRE_LANGUAGE_ENABLE = 1
USE_ITEM_WEAPON_TABLE_ATTACK_BONUS = 0
ADD_DEF_BONUS_ENABLE = 1
LOGIN_COUNT_LIMIT_ENABLE = 0

USE_SKILL_EFFECT_UPGRADE_ENABLE = 1

VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD = 1
GUILD_MONEY_PER_GSP = 100
GUILD_WAR_TYPE_SELECT_ENABLE = 1
TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE = 0

HAIR_COLOR_ENABLE = 1
ARMOR_SPECULAR_ENABLE = 1
WEAPON_SPECULAR_ENABLE = 1
KEEP_ACCOUNT_CONNETION_ENABLE = 1
MINIMAP_POSITIONINFO_ENABLE = 0
CONVERT_EMPIRE_LANGUAGE_ENABLE = 0
USE_ITEM_WEAPON_TABLE_ATTACK_BONUS = 0
ADD_DEF_BONUS_ENABLE = 0
LOGIN_COUNT_LIMIT_ENABLE = 0
PVPMODE_PROTECTED_LEVEL = 15
TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE = 10

isItemDropQuestionDialog = 0

def GET_ITEM_DROP_QUESTION_DIALOG_STATUS():
	global isItemDropQuestionDialog
	return isItemDropQuestionDialog

def SET_ITEM_DROP_QUESTION_DIALOG_STATUS(flag):
	global isItemDropQuestionDialog
	isItemDropQuestionDialog = flag
	
def SET_ITEM_QUESTION_DIALOG_STATUS(flag):
	global isItemQuestionDialog
	isItemQuestionDialog = flag

import app
import net

def SET_DEFAULT_CAMERA_MAX_DISTANCE():
	global CAMERA_MAX_DISTANCE
	app.SetCameraMaxDistance(CAMERA_MAX_DISTANCE)

def SET_CAMERA_MAX_DISTANCE_INDEX(index):
	global CAMERA_MAX_DISTANCE
	global CAMERA_MAX_DISTANCE_LIST
	try:
		CAMERA_MAX_DISTANCE=CAMERA_MAX_DISTANCE_LIST[index]
	except:
		CAMERA_MAX_DISTANCE=CAMERA_MAX_DISTANCE_LIST[0]

	app.SetCameraMaxDistance(CAMERA_MAX_DISTANCE)

def GET_CAMERA_MAX_DISTANCE_INDEX():
	global CAMERA_MAX_DISTANCE
	global CAMERA_MAX_DISTANCE_LIST
	return CAMERA_MAX_DISTANCE_LIST.index(CAMERA_MAX_DISTANCE)

########################

import chrmgr
import player
import app

def SET_DEFAULT_CHRNAME_COLOR():
	global CHRNAME_COLOR_INDEX
	chrmgr.SetEmpireNameMode(CHRNAME_COLOR_INDEX)

def SET_CHRNAME_COLOR_INDEX(index):
	global CHRNAME_COLOR_INDEX
	CHRNAME_COLOR_INDEX=index
	chrmgr.SetEmpireNameMode(index)

def GET_CHRNAME_COLOR_INDEX():
	global CHRNAME_COLOR_INDEX
	return CHRNAME_COLOR_INDEX

def SET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD(index):
	global VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD
	VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD = index

def GET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD():
	global VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD
	return VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD

def SET_DEFAULT_CONVERT_EMPIRE_LANGUAGE_ENABLE():
	global CONVERT_EMPIRE_LANGUAGE_ENABLE
	net.SetEmpireLanguageMode(CONVERT_EMPIRE_LANGUAGE_ENABLE)

def SET_DEFAULT_USE_ITEM_WEAPON_TABLE_ATTACK_BONUS():
	global USE_ITEM_WEAPON_TABLE_ATTACK_BONUS
	player.SetWeaponAttackBonusFlag(USE_ITEM_WEAPON_TABLE_ATTACK_BONUS)

def SET_DEFAULT_USE_SKILL_EFFECT_ENABLE():
	global USE_SKILL_EFFECT_UPGRADE_ENABLE
	app.SetSkillEffectUpgradeEnable(USE_SKILL_EFFECT_UPGRADE_ENABLE)

def SET_TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE():
	global TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE
	app.SetTwoHandedWeaponAttSpeedDecreaseValue(TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE)

########################
import item

ACCESSORY_MATERIAL_LIST = [50623, 50624, 50625, 50626, 50627, 50628, 50629, 50630, 50631, 50632, 50633, 50634, 50635, 50636, 50637, 50638]
#ACCESSORY_MATERIAL_LIST = [50623, 50623, 50624, 50624, 50625, 50625, 50626, 50627, 50628, 50629, 50630, 50631, 50632, 50633, 
#			    50623, 50623, 50624, 50624, ]
JewelAccessoryInfos = [
		# jewel		wrist	neck	ear
		[ 50634,	14420,	16220,	17220 ],	
		[ 50635,	14500,	16500,	17500 ],	
		[ 50636,	14520,	16520,	17520 ],	
		[ 50637,	14540,	16540,	17540 ],	
		[ 50638,	14560,	16560,	17560 ],	
	]
def GET_ACCESSORY_MATERIAL_VNUM(vnum, subType):
	ret = vnum
	item_base = (vnum / 10) * 10
	for info in JewelAccessoryInfos:
		if item.ARMOR_WRIST == subType:	
			if info[1] == item_base:
				return info[0]
		elif item.ARMOR_NECK == subType:	
			if info[2] == item_base:
				return info[0]
		elif item.ARMOR_EAR == subType:	
			if info[3] == item_base:
				return info[0]
 
	if vnum >= 16210 and vnum <= 16219:
		return 50625

	if item.ARMOR_WRIST == subType:	
		WRIST_ITEM_VNUM_BASE = 14000
		ret -= WRIST_ITEM_VNUM_BASE
	elif item.ARMOR_NECK == subType:
		NECK_ITEM_VNUM_BASE = 16000
		ret -= NECK_ITEM_VNUM_BASE
	elif item.ARMOR_EAR == subType:
		EAR_ITEM_VNUM_BASE = 17000
		ret -= EAR_ITEM_VNUM_BASE

	type = ret/20

	if type<0 or type>=len(ACCESSORY_MATERIAL_LIST):
		type = (ret-170) / 20
		if type<0 or type>=len(ACCESSORY_MATERIAL_LIST):
			return 0

	return ACCESSORY_MATERIAL_LIST[type]

##################################################################
## ���� �߰��� '��Ʈ' ������ Ÿ�԰�, ��Ʈ�� ���Ͽ� ���� ������ ����.. 
## ��Ʈ�� ���Ͻý����� �Ǽ������� �����ϱ� ������, �� �Ǽ����� ���� �ϵ��ڵ�ó�� �̷������� �� ���ۿ� ����..

def GET_BELT_MATERIAL_VNUM(vnum, subType = 0):
	# ����� ��� ��Ʈ���� �ϳ��� ������(#18900)�� ���� ����
	return 18900
	
	
## �ڵ����� (HP: #72723 ~ #72726, SP: #72727 ~ #72730)

# �ش� vnum�� �ڵ������ΰ�?
def IS_AUTO_POTION(itemVnum):
	return IS_AUTO_POTION_HP(itemVnum) or IS_AUTO_POTION_SP(itemVnum)
	
# �ش� vnum�� HP �ڵ������ΰ�?
def IS_AUTO_POTION_HP(itemVnum):
	if 72723 <= itemVnum and 72726 >= itemVnum:
		return 1
	elif itemVnum >= 76021 and itemVnum <= 76022:		## ���� �� ������ ȭ���� �ູ
		return 1
	elif itemVnum == 79012:
		return 1
		
	return 0
	
# �ش� vnum�� SP �ڵ������ΰ�?
def IS_AUTO_POTION_SP(itemVnum):
	if 72727 <= itemVnum and 72730 >= itemVnum:
		return 1
	elif itemVnum >= 76004 and itemVnum <= 76005:		## ���� �� ������ ������ �ູ
		return 1
	elif itemVnum == 79013:
		return 1
				
	return 0
def WriteLineInFile(fname, linenum, s):
	import os
	farr = []
	if os.path.exists(fname):
		f = open(fname, "r")
		for line in f:
			farr.append(line)
		f.close()
	while len(farr) < int(linenum):
		farr.append("")
	farr[int(linenum)-1] = str(s)
	f = open(fname, "w")
	for line in farr:
		f.write(line)
		if (len(line) > 0 and line[-1:] != "\n") or len(line) == 0:
			f.write("\n")
	f.close()

def ReadLineInFile(fname, linenum):
	import os
	if not os.path.exists(fname):
		return ""
	f = open(fname, "r")
	farr = []
	for line in f:
		farr.append(line)
	f.close()
	if len(farr) >= int(linenum):
		ret = farr[int(linenum)-1]
		if ret[-1:] == "\n":
			return ret[:-1]
		else:
			return ret
	else:
		return ""

def IS_PET_SEAL(itemVnum):
	item.SelectItem(itemVnum)
	itemType = item.GetItemType()
	return item.ITEM_TYPE_PET == itemType
#pragma once

#include "../GameLib/ItemData.h"

struct SAffects
{
	static constexpr size_t AFFECT_MAX_NUM = 32;
	std::bitset<AFFECT_MAX_NUM> dwAffects;

	SAffects() = default;

	explicit SAffects(uint32_t c_rAffects) : dwAffects(c_rAffects) {}

	SAffects& operator = (const uint32_t& c_rAffects) 
	{
		dwAffects = c_rAffects;
		return *this;
	}

	bool IsAffect(size_t byIndex) const 
	{
		return byIndex < AFFECT_MAX_NUM ? dwAffects.test(byIndex) : false;
	}

	void SetAffects(const unsigned long& c_rAffects) 
	{
		dwAffects = c_rAffects;
	}
};

extern std::string g_strGuildSymbolPathName;

constexpr uint32_t c_Name_Max_Length = 64;
constexpr uint32_t c_FileName_Max_Length = 128;
constexpr uint32_t c_Short_Name_Max_Length = 32;

constexpr uint32_t c_Inventory_Page_Column = 5;
constexpr uint32_t c_Inventory_Page_Row = 9;
constexpr uint32_t c_Inventory_Page_Size = c_Inventory_Page_Column * c_Inventory_Page_Row; // x*y
constexpr uint32_t c_Inventory_Page_Count = 4;

constexpr uint32_t c_ItemSlot_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
constexpr uint32_t c_Equipment_Count = 12;

constexpr uint32_t c_Equipment_Start = c_ItemSlot_Count;

constexpr uint32_t c_Equipment_Body = c_Equipment_Start + CItemData::WEAR_BODY;
constexpr uint32_t c_Equipment_Head = c_Equipment_Start + CItemData::WEAR_HEAD;
constexpr uint32_t c_Equipment_Shoes = c_Equipment_Start + CItemData::WEAR_FOOTS;
constexpr uint32_t c_Equipment_Wrist = c_Equipment_Start + CItemData::WEAR_WRIST;
constexpr uint32_t c_Equipment_Weapon = c_Equipment_Start + CItemData::WEAR_WEAPON;
constexpr uint32_t c_Equipment_Neck = c_Equipment_Start + CItemData::WEAR_NECK;
constexpr uint32_t c_Equipment_Ear = c_Equipment_Start + CItemData::WEAR_EAR;
constexpr uint32_t c_Equipment_Unique1 = c_Equipment_Start + CItemData::WEAR_UNIQUE1;
constexpr uint32_t c_Equipment_Unique2 = c_Equipment_Start + CItemData::WEAR_UNIQUE2;
constexpr uint32_t c_Equipment_Arrow = c_Equipment_Start + CItemData::WEAR_ARROW;
constexpr uint32_t c_Equipment_Shield = c_Equipment_Start + CItemData::WEAR_SHIELD;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
constexpr uint32_t NEW_EQUIPMENT_START = 22;
constexpr uint32_t c_New_Equipment_Start = c_Equipment_Start + NEW_EQUIPMENT_START;
constexpr uint32_t c_New_Equipment_Count = 3;
constexpr uint32_t c_Equipment_ring1 = c_Equipment_Start + CItemData::WEAR_RING1;
constexpr uint32_t c_Equipment_ring2 = c_Equipment_Start + CItemData::WEAR_RING2;
constexpr uint32_t c_Equipment_Belt = c_Equipment_Start + CItemData::WEAR_BELT;
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1 = 0,
	DS_DECK_2 = 1,
	DS_DECK_MAX_NUM = 2,
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL = 0,
	DRAGON_SOUL_GRADE_BRILLIANT = 1,
	DRAGON_SOUL_GRADE_RARE = 2,
	DRAGON_SOUL_GRADE_ANCIENT = 3,
	DRAGON_SOUL_GRADE_LEGENDARY = 4,
	DRAGON_SOUL_GRADE_MAX = 5,
};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST = 0,
	DRAGON_SOUL_STEP_LOW = 1,
	DRAGON_SOUL_STEP_MID = 2,
	DRAGON_SOUL_STEP_HIGH = 3,
	DRAGON_SOUL_STEP_HIGHEST = 4,
	DRAGON_SOUL_STEP_MAX = 5,
};

constexpr uint32_t c_Costume_Slot_Start = c_Equipment_Start + CItemData::WEAR_COSTUME_BODY;
constexpr uint32_t c_Costume_Slot_Body = c_Costume_Slot_Start + CItemData::COSTUME_BODY;
constexpr uint32_t c_Costume_Slot_Hair = c_Costume_Slot_Start + CItemData::COSTUME_HAIR;
constexpr uint32_t c_Costume_Slot_Mount = c_Costume_Slot_Start + CItemData::COSTUME_MOUNT;
constexpr uint32_t c_Costume_Slot_Count = 3;
constexpr uint32_t c_Costume_Slot_End = c_Costume_Slot_Start + c_Costume_Slot_Count;

constexpr uint32_t c_Wear_Max = CItemData::WEAR_MAX_NUM;
constexpr uint32_t c_DragonSoul_Equip_Start = c_ItemSlot_Count + c_Wear_Max;
constexpr uint32_t c_DragonSoul_Equip_Slot_Max = 6;
constexpr uint32_t c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;
constexpr uint32_t c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
constexpr uint32_t c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
constexpr uint32_t c_Belt_Inventory_Width = 4;
constexpr uint32_t c_Belt_Inventory_Height= 4;
constexpr uint32_t c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
constexpr uint32_t c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;
constexpr uint32_t c_Inventory_Count = c_Belt_Inventory_Slot_End;
#else
const uint32_t c_Inventory_Count = c_DragonSoul_Equip_End;
#endif

constexpr uint32_t c_DragonSoul_Inventory_Start = 0;
constexpr uint32_t c_DragonSoul_Inventory_Box_Size = 32;
constexpr uint32_t c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
constexpr uint32_t c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

enum ESlotType
{
	SLOT_TYPE_NONE = 0,
	SLOT_TYPE_INVENTORY = 1,
	SLOT_TYPE_SKILL = 2,
	SLOT_TYPE_EMOTION = 3,
	SLOT_TYPE_SHOP = 4,
	SLOT_TYPE_EXCHANGE_OWNER = 5,
	SLOT_TYPE_EXCHANGE_TARGET = 6,
	SLOT_TYPE_QUICK_SLOT = 7,
	SLOT_TYPE_SAFEBOX = 8,
	SLOT_TYPE_PRIVATE_SHOP = 9,
	SLOT_TYPE_MALL = 10,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY = 11,
	SLOT_TYPE_MAX = 12,
};

enum EWindows
{
	RESERVED_WINDOW = 0,
	INVENTORY = 1,
	EQUIPMENT = 2,
	SAFEBOX = 3,
	MALL = 4,
	DRAGON_SOUL_INVENTORY = 5,
	GROUND = 6,
	BELT_INVENTORY = 7,
	WINDOW_TYPE_MAX = 8,
};

enum EKeySettings
{
	KEY_NONE = 0,
	KEY_MOVE_UP_1 = 1,
	KEY_MOVE_DOWN_1 = 2,
	KEY_MOVE_LEFT_1 = 3,
	KEY_MOVE_RIGHT_1 = 4,
	KEY_MOVE_UP_2 = 5,
	KEY_MOVE_DOWN_2 = 6,
	KEY_MOVE_LEFT_2 = 7,
	KEY_MOVE_RIGHT_2 = 8,
	KEY_CAMERA_ROTATE_POSITIVE_1 = 9,
	KEY_CAMERA_ROTATE_NEGATIVE_1 = 10,
	KEY_CAMERA_ZOOM_POSITIVE_1 = 11,
	KEY_CAMERA_ZOOM_NEGATIVE_1 = 12,
	KEY_CAMERA_PITCH_POSITIVE_1 = 13,
	KEY_CAMERA_PITCH_NEGATIVE_1 = 14,
	KEY_CAMERA_ROTATE_POSITIVE_2 = 15,
	KEY_CAMERA_ROTATE_NEGATIVE_2 = 16,
	KEY_CAMERA_ZOOM_POSITIVE_2 = 17,
	KEY_CAMERA_ZOOM_NEGATIVE_2 = 18,
	KEY_CAMERA_PITCH_POSITIVE_2 = 19,
	KEY_CAMERA_PITCH_NEGATIVE_2 = 20,
	KEY_ROOTING_1 = 21,
	KEY_ROOTING_2 = 22,
	KEY_ATTACK = 23,
	KEY_RIDEMYHORS = 24,
	KEY_FEEDMYHORS = 25,
	KEY_BYEMYHORS = 26,
	KEY_RIDEHORS = 27,
	KEY_EMOTION1 = 28,
	KEY_EMOTION2 = 29,
	KEY_EMOTION3 = 30,
	KEY_EMOTION4 = 31,
	KEY_EMOTION5 = 32,
	KEY_EMOTION6 = 33,
	KEY_EMOTION7 = 34,
	KEY_EMOTION8 = 35,
	KEY_EMOTION9 = 36,
	KEY_SLOT_1 = 37,
	KEY_SLOT_2 = 38,
	KEY_SLOT_3 = 39,
	KEY_SLOT_4 = 40,
	KEY_SLOT_5 = 41,
	KEY_SLOT_6 = 42,
	KEY_SLOT_7 = 43,
	KEY_SLOT_8 = 44,
	KEY_SLOT_CHANGE_1 = 45,
	KEY_SLOT_CHANGE_2 = 46,
	KEY_SLOT_CHANGE_3 = 47,
	KEY_SLOT_CHANGE_4 = 48,
	KEY_OPEN_STATE = 49,
	KEY_OPEN_SKILL = 50,
	KEY_OPEN_QUEST = 51,
	KEY_OPEN_INVENTORY = 52,
	KEY_OPEN_DDS = 53,
	KEY_OPEN_MINIMAP = 54,
	KEY_OPEN_LOGCHAT = 55,
	KEY_OPEN_GUILD = 56,
	KEY_OPEN_MESSENGER = 57,
	KEY_OPEN_HELP = 58,
	KEY_OPEN_ACTION = 59,
	KEY_SCROLL_ONOFF = 60,
	KEY_PLUS_MINIMAP = 61,
	KEY_MIN_MINIMAP = 62,
	KEY_SCREENSHOT = 63,
	KEY_SHOW_NAME = 64,
	KEY_AUTO_RUN = 65,
	KEY_NEXT_TARGET = 66,
	KEY_PASSIVE_ATTR1 = 67,
	KEY_PASSIVE_ATTR2 = 68,

	KEY_ADDKEYBUFFERCONTROL = 100,
	KEY_ADDKEYBUFFERALT = 300,
	KEY_ADDKEYBUFFERSHIFT = 500,
};

#pragma pack (push, 1)
typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;

	SItemPos()
	{
		window_type = INVENTORY;
		cell = 0xffff;
	}

	SItemPos (BYTE _window_type, WORD _cell)
	{
		window_type = _window_type;
		cell = _cell;
	}

	bool IsValidCell() const
	{
		switch (window_type)
		{
			case INVENTORY:
				return cell < c_Inventory_Count;

			case EQUIPMENT:
				return cell < c_DragonSoul_Equip_End;

			case DRAGON_SOUL_INVENTORY:
				return cell < (DS_INVENTORY_MAX_NUM);

			default:
				return false;
		}
	}

	bool IsEquipCell() const
	{
		switch (window_type)
		{
			case INVENTORY:
			case EQUIPMENT:
				return (cell >= c_Equipment_Start) && (cell < c_Equipment_Start + c_Wear_Max);

			case BELT_INVENTORY:
			case DRAGON_SOUL_INVENTORY:
				return false;

			default:
				return false;
		}
	}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell() const
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
#endif

	bool operator == (const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator < (const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}

} TItemPos;
#pragma pack(pop)

constexpr uint32_t c_QuickBar_Line_Count = 3;
constexpr uint32_t c_QuickBar_Slot_Count = 12;

constexpr float c_Idle_WaitTime = 5.0f;

constexpr int32_t c_Monster_Race_Start_Number = 6;
constexpr int32_t c_Monster_Model_Start_Number = 20001;

constexpr float c_fAttack_Delay_Time = 0.2f;
constexpr float c_fHit_Delay_Time = 0.1f;
constexpr float c_fCrash_Wave_Time = 0.2f;
constexpr float c_fCrash_Wave_Distance = 3.0f;
constexpr float c_fHeight_Step_Distance = 50.0f;

enum EDistanceType
{
	DISTANCE_TYPE_FOUR_WAY = 0,
	DISTANCE_TYPE_EIGHT_WAY = 1,
	DISTANCE_TYPE_ONE_WAY = 2,
	DISTANCE_TYPE_MAX_NUM = 3,
};

constexpr float c_fMagic_Script_Version = 1.0f;
constexpr float c_fSkill_Script_Version = 1.0f;
constexpr float c_fMagicSoundInformation_Version = 1.0f;
constexpr float c_fBattleCommand_Script_Version = 1.0f;
constexpr float c_fEmotionCommand_Script_Version = 1.0f;
constexpr float c_fActive_Script_Version = 1.0f;
constexpr float c_fPassive_Script_Version = 1.0f;

constexpr float c_fWalkDistance = 175.0f;
constexpr float c_fRunDistance = 310.0f;

constexpr uint32_t FILE_MAX_LEN = 128;

enum EItemAttribute
{
	ITEM_SOCKET_SLOT_MAX_NUM = 3,
	ITEM_ATTRIBUTE_SLOT_MAX_NUM = 7,
};

#pragma pack(push)
#pragma pack(1)
typedef struct SQuickSlot
{
	BYTE Type;
	BYTE Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
	BYTE bType;
	short sValue;
} TPlayerItemAttribute;

typedef struct packet_item
{
	uint32_t vnum;
	uint8_t count;
	uint32_t flags;
	uint32_t anti_flags;
	int32_t alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TItemData;

typedef struct packet_shop_item
{
	DWORD vnum;
	DWORD price;
	BYTE count;
	BYTE display_pos;
	int32_t alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TShopItemData;
#pragma pack(pop)

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2)
{
	auto dx = ix1 - ix2;
	auto dy = iy1 - iy2;

	return std::sqrt(static_cast<float>(dx * dx + dy * dy));
}

void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char* c_szFontName);

CResource* DefaultFont_GetResource();
CResource* DefaultItalicFont_GetResource();

void SetGuildSymbolPath(const char* c_szPathName);
const char* GetGuildSymbolFileName(DWORD dwGuildID);
BYTE SlotTypeToInvenType(BYTE bSlotType);
#pragma once

#include "../eterLib/GrpSubImage.h"
#include "../eterGrnLib/Thing.h"
#include "GameType.h"

class CItemData
{
	public:
		enum
		{
			ITEM_NAME_MAX_LEN = 32,
			ITEM_LIMIT_MAX_NUM = 2,
			ITEM_VALUES_MAX_NUM = 6,
			ITEM_SMALL_DESCR_MAX_LEN = 256,
			ITEM_APPLY_MAX_NUM = 3,
			ITEM_SOCKET_MAX_NUM = 3,
		};

		enum EItemType
		{
			ITEM_TYPE_NONE = 0,
			ITEM_TYPE_WEAPON = 1,
			ITEM_TYPE_ARMOR = 2,
			ITEM_TYPE_USE = 3,
			ITEM_TYPE_AUTOUSE = 4,
			ITEM_TYPE_MATERIAL = 5,
			ITEM_TYPE_SPECIAL = 6,
			ITEM_TYPE_TOOL = 7,
			ITEM_TYPE_LOTTERY = 8,
			ITEM_TYPE_ELK = 9,
			ITEM_TYPE_METIN = 10,
			ITEM_TYPE_CONTAINER = 11,
			ITEM_TYPE_FISH = 12,
			ITEM_TYPE_ROD = 13,
			ITEM_TYPE_RESOURCE = 14,
			ITEM_TYPE_CAMPFIRE = 15,
			ITEM_TYPE_UNIQUE = 16,
			ITEM_TYPE_SKILLBOOK = 17,
			ITEM_TYPE_QUEST = 18,
			ITEM_TYPE_POLYMORPH = 19,
			ITEM_TYPE_TREASURE_BOX = 20,
			ITEM_TYPE_TREASURE_KEY = 21, 
			ITEM_TYPE_SKILLFORGET = 22,
			ITEM_TYPE_GIFTBOX = 23,
			ITEM_TYPE_PICK = 24,
			ITEM_TYPE_HAIR = 25,
			ITEM_TYPE_TOTEM = 26,
			ITEM_TYPE_BLEND = 27,
			ITEM_TYPE_COSTUME = 28,
			ITEM_TYPE_DS = 29,
			ITEM_TYPE_SPECIAL_DS = 30,
			ITEM_TYPE_EXTRACT = 31,
			ITEM_TYPE_SECONDARY_COIN = 32,
			ITEM_TYPE_RING = 33,
			ITEM_TYPE_BELT = 34,
			ITEM_TYPE_GACHA = 35,
			ITEM_TYPE_PET = 36,
			ITEM_TYPE_MAX_NUM = 37,
		};

		enum EMetinSubTypes
		{
			METIN_NORMAL = 0,
			METIN_GOLD = 1,
		};

		enum EWeaponSubTypes
		{
			WEAPON_SWORD = 0,
			WEAPON_DAGGER = 1,
			WEAPON_BOW = 2,
			WEAPON_TWO_HANDED = 3,
			WEAPON_BELL = 4,
			WEAPON_FAN = 5,
			WEAPON_ARROW = 6,
			WEAPON_MOUNT_SPEAR = 7,
			WEAPON_NUM_TYPES = 8,
			WEAPON_NONE = WEAPON_NUM_TYPES + 1,
		};

		enum EArmorSubTypes
		{
			ARMOR_BODY = 0,
			ARMOR_HEAD = 1,
			ARMOR_SHIELD = 2,
			ARMOR_WRIST = 3,
			ARMOR_FOOTS = 4,
			ARMOR_NECK = 5,
			ARMOR_EAR = 6,
			ARMOR_NUM_TYPES = 7,
		};

		enum ECostumeSubTypes
		{
			COSTUME_BODY = 0,
			COSTUME_HAIR = 1,
			COSTUME_MOUNT = 2,
			COSTUME_NUM_TYPES = 3,
		};

		enum EDragonSoulSubType
		{
			DS_SLOT1 = 0,
			DS_SLOT2 = 1,
			DS_SLOT3 = 2,
			DS_SLOT4 = 3,
			DS_SLOT5 = 4,
			DS_SLOT6 = 5,
			DS_SLOT_NUM_TYPES = 6,
		};

		enum EUseSubTypes
		{
			USE_POTION = 0,
			USE_TALISMAN = 1,
			USE_TUNING = 2,
			USE_MOVE = 3,
			USE_TREASURE_BOX = 4,
			USE_MONEYBAG = 5,
			USE_BAIT = 6,
			USE_ABILITY_UP = 7,
			USE_AFFECT = 8,
			USE_CREATE_STONE = 9,
			USE_SPECIAL = 10,
			USE_POTION_NODELAY = 11,
			USE_CLEAR = 12,
			USE_INVISIBILITY = 13,
			USE_DETACHMENT = 14,
			USE_BUCKET = 15,
			USE_POTION_CONTINUE = 16,
			USE_CLEAN_SOCKET = 17,
			USE_CHANGE_ATTRIBUTE = 18,
			USE_ADD_ATTRIBUTE = 19,
			USE_ADD_ACCESSORY_SOCKET = 20,
			USE_PUT_INTO_ACCESSORY_SOCKET = 21,
			USE_ADD_ATTRIBUTE2 = 22,
			USE_RECIPE = 23,
			USE_CHANGE_ATTRIBUTE2 = 24,
			USE_BIND = 25,
			USE_UNBIND = 26,
			USE_TIME_CHARGE_PER = 27,
			USE_TIME_CHARGE_FIX = 28,
			USE_PUT_INTO_BELT_SOCKET = 29,
			USE_PUT_INTO_RING_SOCKET = 30,
		};

		enum EMaterialSubTypes
		{
			MATERIAL_LEATHER = 0,
			MATERIAL_BLOOD = 1,
			MATERIAL_ROOT = 2,
			MATERIAL_NEEDLE = 3,
			MATERIAL_JEWEL = 4,
			MATERIAL_DS_REFINE_NORMAL = 5,
			MATERIAL_DS_REFINE_BLESSED = 6,
			MATERIAL_DS_REFINE_HOLLY = 7,
		};

		enum EItemAntiFlag
		{
			ITEM_ANTIFLAG_FEMALE = (1 << 0),
			ITEM_ANTIFLAG_MALE = (1 << 1),
			ITEM_ANTIFLAG_WARRIOR = (1 << 2),
			ITEM_ANTIFLAG_ASSASSIN = (1 << 3),
			ITEM_ANTIFLAG_SURA = (1 << 4),
			ITEM_ANTIFLAG_SHAMAN = (1 << 5),
			ITEM_ANTIFLAG_GET = (1 << 6),
			ITEM_ANTIFLAG_DROP = (1 << 7),
			ITEM_ANTIFLAG_SELL = (1 << 8),
			ITEM_ANTIFLAG_EMPIRE_A = (1 << 9),
			ITEM_ANTIFLAG_EMPIRE_B = (1 << 10),
			ITEM_ANTIFLAG_EMPIRE_R = (1 << 11),
			ITEM_ANTIFLAG_SAVE = (1 << 12),
			ITEM_ANTIFLAG_GIVE = (1 << 13),
			ITEM_ANTIFLAG_PKDROP = (1 << 14),
			ITEM_ANTIFLAG_STACK = (1 << 15),
			ITEM_ANTIFLAG_MYSHOP = (1 << 16),
			ITEM_ANTIFLAG_SAFEBOX = (1 << 17),
		};

		enum EItemFlag
		{
			ITEM_FLAG_REFINEABLE = (1 << 0),
			ITEM_FLAG_SAVE = (1 << 1),
			ITEM_FLAG_STACKABLE = (1 << 2),
			ITEM_FLAG_COUNT_PER_1GOLD = (1 << 3),
			ITEM_FLAG_SLOW_QUERY = (1 << 4),
			ITEM_FLAG_RARE = (1 << 5),
			ITEM_FLAG_UNIQUE = (1 << 6),
			ITEM_FLAG_MAKECOUNT = (1 << 7),
			ITEM_FLAG_IRREMOVABLE = (1 << 8),
			ITEM_FLAG_CONFIRM_WHEN_USE = (1 << 9),
			ITEM_FLAG_QUEST_USE = (1 << 10),
			ITEM_FLAG_QUEST_USE_MULTIPLE = (1 << 11),
			ITEM_FLAG_UNUSED03 = (1 << 12),
			ITEM_FLAG_LOG = (1 << 13),
			ITEM_FLAG_APPLICABLE = (1 << 14),
		};

		enum EWearPositions
		{
			WEAR_BODY = 0,
			WEAR_HEAD = 1,
			WEAR_FOOTS = 2,
			WEAR_WRIST = 3,
			WEAR_WEAPON = 4,
			WEAR_NECK = 5,
			WEAR_EAR = 6,
			WEAR_UNIQUE1 = 7,
			WEAR_UNIQUE2 = 8,
			WEAR_ARROW = 9,
			WEAR_SHIELD = 10,
			WEAR_ABILITY1 = 11,
			WEAR_ABILITY2 = 12,
			WEAR_ABILITY3 = 13,
			WEAR_ABILITY4 = 14,
			WEAR_ABILITY5 = 15,
			WEAR_ABILITY6 = 16,
			WEAR_ABILITY7 = 17,
			WEAR_ABILITY8 = 18,
			WEAR_COSTUME_BODY = 19,
			WEAR_COSTUME_HAIR = 20,
			WEAR_COSTUME_MOUNT = 21,
			WEAR_RING1 = 22,
			WEAR_RING2 = 23,
			WEAR_BELT = 24,
			WEAR_MAX_NUM = 25,
		};

		enum EItemWearableFlag
		{
			WEARABLE_BODY = (1 << 0),
			WEARABLE_HEAD = (1 << 1),
			WEARABLE_FOOTS = (1 << 2),
			WEARABLE_WRIST = (1 << 3),
			WEARABLE_WEAPON = (1 << 4),
			WEARABLE_NECK = (1 << 5),
			WEARABLE_EAR = (1 << 6),
			WEARABLE_UNIQUE = (1 << 7),
			WEARABLE_SHIELD = (1 << 8),
			WEARABLE_ARROW = (1 << 9),
			WEARABLE_HAIR = (1 << 10),
			WEARABLE_ABILITY = (1 << 11),
			WEARABLE_COSTUME_BODY = (1 << 12),
		};

		enum ELimitTypes
		{
			LIMIT_NONE = 0,
			LIMIT_LEVEL = 1,
			LIMIT_STR = 2,
			LIMIT_DEX = 3,
			LIMIT_INT = 4,
			LIMIT_CON = 5,
			LIMIT_REAL_TIME = 7,
			LIMIT_REAL_TIME_START_FIRST_USE = 8,
			LIMIT_TIMER_BASED_ON_WEAR = 9,
			LIMIT_MAX_NUM = 10,
		};

		enum EApplyTypes
		{
			APPLY_NONE = 0,
			APPLY_MAX_HP = 1,
			APPLY_MAX_SP = 2,
			APPLY_CON = 3,
			APPLY_INT = 4,
			APPLY_STR = 5,
			APPLY_DEX = 6,
			APPLY_ATT_SPEED = 7,
			APPLY_MOV_SPEED = 8,
			APPLY_CAST_SPEED = 9,
			APPLY_HP_REGEN = 10,
			APPLY_SP_REGEN = 11,
			APPLY_POISON_PCT = 12,
			APPLY_STUN_PCT = 13,
			APPLY_SLOW_PCT = 14,
			APPLY_CRITICAL_PCT = 15,
			APPLY_PENETRATE_PCT = 16,
			APPLY_ATTBONUS_HUMAN = 17,
			APPLY_ATTBONUS_ANIMAL = 18,
			APPLY_ATTBONUS_ORC = 19,
			APPLY_ATTBONUS_MILGYO = 20,
			APPLY_ATTBONUS_UNDEAD = 21,
			APPLY_ATTBONUS_DEVIL = 22,
			APPLY_STEAL_HP = 23,
			APPLY_STEAL_SP = 24,
			APPLY_MANA_BURN_PCT = 25,
			APPLY_DAMAGE_SP_RECOVER = 26,
			APPLY_BLOCK = 27,
			APPLY_DODGE = 28,
			APPLY_RESIST_SWORD = 29,
			APPLY_RESIST_TWOHAND = 30,
			APPLY_RESIST_DAGGER = 31,
			APPLY_RESIST_BELL = 32,
			APPLY_RESIST_FAN = 33,
			APPLY_RESIST_BOW = 34,
			APPLY_RESIST_FIRE = 35,
			APPLY_RESIST_ELEC = 36,
			APPLY_RESIST_MAGIC = 37,
			APPLY_RESIST_WIND = 38,
			APPLY_REFLECT_MELEE = 39,
			APPLY_REFLECT_CURSE = 40,
			APPLY_POISON_REDUCE = 41,
			APPLY_KILL_SP_RECOVER = 42,
			APPLY_EXP_DOUBLE_BONUS = 43,
			APPLY_GOLD_DOUBLE_BONUS = 44,
			APPLY_ITEM_DROP_BONUS = 45,
			APPLY_POTION_BONUS = 46,
			APPLY_KILL_HP_RECOVER = 47,
			APPLY_IMMUNE_STUN = 48,
			APPLY_IMMUNE_SLOW = 49,
			APPLY_IMMUNE_FALL = 50,
			APPLY_SKILL = 51,
			APPLY_BOW_DISTANCE = 52,
			APPLY_ATT_GRADE_BONUS = 53,
			APPLY_DEF_GRADE_BONUS = 54,
			APPLY_MAGIC_ATT_GRADE = 55,
			APPLY_MAGIC_DEF_GRADE = 56,
			APPLY_CURSE_PCT = 57,
			APPLY_MAX_STAMINA = 58,
			APPLY_ATTBONUS_WARRIOR = 59,
			APPLY_ATTBONUS_ASSASSIN = 60,
			APPLY_ATTBONUS_SURA = 61,
			APPLY_ATTBONUS_SHAMAN = 62,
			APPLY_ATTBONUS_MONSTER = 63,
			APPLY_MALL_ATTBONUS = 64,
			APPLY_MALL_DEFBONUS = 65,
			APPLY_MALL_EXPBONUS = 66,
			APPLY_MALL_ITEMBONUS = 67,
			APPLY_MALL_GOLDBONUS = 68,
			APPLY_MAX_HP_PCT = 69,
			APPLY_MAX_SP_PCT = 70,
			APPLY_SKILL_DAMAGE_BONUS = 71,
			APPLY_NORMAL_HIT_DAMAGE_BONUS = 72,
			APPLY_SKILL_DEFEND_BONUS = 73,
			APPLY_NORMAL_HIT_DEFEND_BONUS = 74,
			APPLY_EXTRACT_HP_PCT = 75,
			APPLY_RESIST_WARRIOR = 76,
			APPLY_RESIST_ASSASSIN = 77,
			APPLY_RESIST_SURA = 78,
			APPLY_RESIST_SHAMAN = 79,
			APPLY_ENERGY = 80,
			APPLY_DEF_GRADE = 81,
			APPLY_COSTUME_ATTR_BONUS = 82,
			APPLY_MAGIC_ATTBONUS_PER = 83,
			APPLY_MELEE_MAGIC_ATTBONUS_PER = 84,
			APPLY_RESIST_ICE = 85,
			APPLY_RESIST_EARTH = 86,
			APPLY_RESIST_DARK = 87,
			APPLY_ANTI_CRITICAL_PCT = 88,
			APPLY_ANTI_PENETRATE_PCT = 89,
			MAX_APPLY_NUM = 90,
		};

		enum EImmuneFlags
		{
			IMMUNE_PARA = (1 << 0),
			IMMUNE_CURSE = (1 << 1),
			IMMUNE_STUN = (1 << 2),
			IMMUNE_SLEEP = (1 << 3),
			IMMUNE_SLOW = (1 << 4),
			IMMUNE_POISON = (1 << 5),
			IMMUNE_TERROR = (1 << 6),
		};

		enum EPetSubTypes
		{
			PET_PAY,
		};

#pragma pack(push)
#pragma pack(1)
		typedef struct SItemLimit
		{
			BYTE bType;
			long lValue;
		} TItemLimit;

		typedef struct SItemApply
		{
			BYTE bType;
			long lValue;
		} TItemApply;

		typedef struct SItemTable
		{
			uint32_t dwVnum;
			uint32_t dwVnumRange;
			char szName[ITEM_NAME_MAX_LEN + 1];
			char szLocaleName[ITEM_NAME_MAX_LEN + 1];
			uint8_t bType;
			uint8_t bSubType;
			uint8_t bWeight;
			uint8_t	bSize;
			uint32_t dwAntiFlags;
			uint32_t dwFlags;
			uint32_t dwWearFlags;
			uint32_t dwImmuneFlag;
			uint32_t dwIBuyItemPrice;
			uint32_t dwISellItemPrice;
			TItemLimit aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply aApplies[ITEM_APPLY_MAX_NUM];
			int32_t alValues[ITEM_VALUES_MAX_NUM];
			int32_t alSockets[ITEM_SOCKET_MAX_NUM];
			uint32_t dwRefinedVnum;
			uint16_t wRefineSet;
			uint8_t bAlterToMagicItemPct;
			uint8_t bSpecular;
			uint8_t bGainSocketPct;
			int16_t sAddonType;
			char cLimitRealTimeFirstUseIndex;
			char cLimitTimerBasedOnWearIndex;
		} TItemTable;
#pragma pack(pop)

	public:
		CItemData();
		virtual ~CItemData();

		void Clear();
		void SetSummary(const std::string& c_rstSumm);
		void SetDescription(const std::string& c_rstDesc);

		CGraphicThing* GetModelThing();
		CGraphicThing* GetSubModelThing();
		CGraphicThing* GetDropModelThing();
		CGraphicSubImage* GetIconImage();

		DWORD GetLODModelThingCount();
		BOOL GetLODModelThingPointer(DWORD dwIndex, CGraphicThing** ppModelThing);

		DWORD GetAttachingDataCount();
		BOOL GetCollisionDataPointer(DWORD dwIndex, const NRaceData::TAttachingData** c_ppAttachingData);
		BOOL GetAttachingDataPointer(DWORD dwIndex, const NRaceData::TAttachingData** c_ppAttachingData);

		const TItemTable* GetTable() const;
		DWORD GetIndex() const;
		const char* GetName() const;
		const char* GetDescription() const;
		const char* GetSummary() const;
		BYTE GetType() const;
		BYTE GetSubType() const;
		UINT GetRefine() const;
		const char* GetUseTypeString() const;
		DWORD GetWeaponType() const;
		BYTE GetSize() const;
		BOOL IsAntiFlag(DWORD dwFlag) const;
		BOOL IsFlag(DWORD dwFlag) const;
		BOOL IsWearableFlag(DWORD dwFlag) const;
		BOOL HasNextGrade() const;
		DWORD GetWearFlags() const;
		DWORD GetIBuyItemPrice() const;
		DWORD GetISellItemPrice() const;
		BOOL GetLimit(BYTE byIndex, TItemLimit* pItemLimit) const;
		BOOL GetApply(BYTE byIndex, TItemApply* pItemApply) const;
		long GetValue(BYTE byIndex) const;
		long GetSocket(BYTE byIndex) const;
		long SetSocket(BYTE byIndex,DWORD value);
		int GetSocketCount() const;
		DWORD GetIconNumber() const;
		UINT GetSpecularPoweru() const;
		float GetSpecularPowerf() const;
		BOOL IsEquipment() const;

		void SetDefaultItemData(const char* c_szIconFileName, const char* c_szModelFileName = NULL);
		void SetItemTableData(TItemTable* pItemTable);

	protected:
		void __LoadFiles();
		void __SetIconImage(const char* c_szFileName);

	protected:
		std::string m_strModelFileName;
		std::string m_strSubModelFileName;
		std::string m_strDropModelFileName;
		std::string m_strIconFileName;
		std::string m_strDescription;
		std::string m_strSummary;
		std::vector<std::string> m_strLODModelFileNameVector;

		CGraphicThing* m_pModelThing;
		CGraphicThing* m_pSubModelThing;
		CGraphicThing* m_pDropModelThing;
		CGraphicSubImage* m_pIconImage;
		std::vector<CGraphicThing*> m_pLODModelThingVector;

		NRaceData::TAttachingDataVector m_AttachingDataVector;
		DWORD m_dwVnum;
		TItemTable m_ItemTable;

	public:
		static void DestroySystem();
		static CItemData* New();
		static void Delete(CItemData* pkItemData);
		static CDynamicPool<CItemData> ms_kPool;
};

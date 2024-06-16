#pragma once
#include "entity.h"

class CItem : public CEntity
{
	protected:
		virtual void EncodeInsertPacket(LPENTITY entity);
		virtual void EncodeRemovePacket(LPENTITY entity);

	public:
		CItem(uint32_t dwVnum);
		virtual ~CItem();

		void Initialize();
		void Destroy();

		void SetDestroyEvent(LPEVENT pkEvent);
		void StartDestroyEvent(int32_t iSec = 300);

		void SetProto(const TItemTable* table);
		
		void UsePacketEncode(LPCHARACTER character, LPCHARACTER targetCharacter, struct packet_item_use* usePacket);

		void AddFlag(int32_t bit);
		void RemoveFlag(int32_t bit);

		uint32_t GetWearFlag()
		{ 
			return m_pProto ? m_pProto->dwWearFlags : 0; 
		}

		void UpdatePacket();
		
		bool SetCount(uint32_t count);
		uint32_t GetCount() const;

		bool AddToCharacter(LPCHARACTER character, TItemPos cellPosition);
		LPITEM RemoveFromCharacter();

		bool AddToGround(int32_t mapIndex, const PIXEL_POSITION& position, bool skipOwnerCheck = false);
		LPITEM RemoveFromGround();

		bool DistanceValid(LPCHARACTER character);
		bool CanCharacterEquipItem(LPCHARACTER character);

		int32_t FindEquipCell(LPCHARACTER character, int32_t bCandidateCell = -1);

	private:
		bool IsEquippableItemType(EItemTypes itemType);
		bool IsItemTypeDragonSoul(EItemTypes itemType);
		int32_t CalculateDragonSoulCell(int32_t iCandidateCell);
		int32_t CalculateCostumeEquipCell();
		int32_t CalculateRingEquipCell(LPCHARACTER character);
		int32_t CalculateEquipCellByWearFlag(int32_t wearFlag, LPCHARACTER character);
		int32_t CalculateAbilityEquipCell(LPCHARACTER character);

		void AssignToOwner(LPCHARACTER newOwner, uint8_t wearCell);
		void ApplyImmuneFlag();
		void ApplyItemEffects();

	public:	
		void ModifyPoints(bool bAdd, LPCHARACTER character = nullptr);
		
		bool IsEquipable() const;
		bool EquipTo(LPCHARACTER character, uint8_t wearCell);
		bool Unequip();

		int32_t GetValue(uint32_t idx);

		void SetExchanging(bool isOn = true);
		void Save();

		bool CreateSocket(uint8_t bSlot, uint8_t bGold);
		void SetSockets(const int32_t* al);
		void SetSocket(int32_t i, int32_t v, bool bLog = true);

		int32_t GetGold();
		int32_t GetShopBuyPrice();

		bool IsOwnership(LPCHARACTER character);
		void SetOwnership(LPCHARACTER character, int32_t iSec = 10);
		void SetOwnershipEvent(LPEVENT pkEvent);

		int32_t GetSocketCount();
		bool AddSocket();
		
		void AlterToSocketItem(int32_t iSocketCount);
		void AlterToMagicItem();

		uint32_t GetRefineFromVnum();
		int32_t GetRefineLevel();

		bool IsPolymorphItem();

		void SetUniqueExpireEvent(LPEVENT pkEvent);
		void SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent);
		void StartRealTimeExpireEvent();
		bool IsRealTimeItem() const;
		void StopUniqueExpireEvent();
		void StopTimerBasedOnWearExpireEvent();
		void StartUniqueExpireEvent();
		void StartTimerBasedOnWearExpireEvent();

		void ApplyAddon(int32_t iAddonType);
		int32_t GetSpecialGroup() const;

		bool IsAccessoryForSocket();
		void SetAccessorySocketGrade(int32_t iGrade);
		void SetAccessorySocketMaxGrade(int32_t iMaxGrade);
		void SetAccessorySocketDownGradeTime(uint32_t time);
		
		void StartAccessorySocketExpireEvent();
		void StopAccessorySocketExpireEvent();
		void SetAccessorySocketExpireEvent(LPEVENT pkEvent);
		void AccessorySocketDegrade();

		bool IsRideItem();
		bool IsRamadanRing();

		void ClearMountAttributeAndAffect();
		bool IsNewMountItem();
		
		bool CanPutInto(LPITEM item);
		bool CheckItemUseLevel(int32_t nLevel) const;
		int32_t FindApplyValue(uint8_t bApplyType) const;
		void CopySocketTo(LPITEM pItem);

		int32_t GetAccessorySocketGrade();
		int32_t GetAccessorySocketMaxGrade();
		int32_t GetAccessorySocketDownGradeTime();

		void AttrLog() const;
		int32_t GetLevelLimit() const;

		bool IsDragonSoul() const;
		int32_t GiveMoreTime_Per(float fPercent);
		int32_t GiveMoreTime_Fix(uint32_t dwTime);

		int32_t GetDuration();
		bool IsSameSpecialGroup(const LPITEM item) const;

	public:
		int32_t GetAttributeSetIndex();
		bool HasAttr(uint8_t bApply);
		bool HasRareAttr(uint8_t bApply);
		void ChangeAttribute(const int32_t* aiChangeProb = NULL);
		void AddAttribute();
		void AddAttribute (uint8_t bType, short sValue);
		void ClearAttribute();
		int32_t GetAttributeCount();
		int32_t FindAttribute(uint8_t bType);
		bool RemoveAttributeAt(int32_t index);
		bool RemoveAttributeType(uint8_t bType);		
		void SetAttributes(const TPlayerItemAttribute* c_pAttribute);
		void SetForceAttribute(int32_t i, uint8_t bType, short sValue);
		void CopyAttributeTo(LPITEM pItem);
		int32_t GetRareAttrCount();
		bool ChangeRareAttribute();
		bool AddRareAttribute();

	protected:
		void AddAttr(uint8_t bApply, uint8_t bLevel);
		void PutAttribute(const int32_t * aiAttrPercentTable);
		void PutAttributeWithLevel(uint8_t bLevel);		
		void SetAttribute(int32_t i, uint8_t bType, short sValue);

	public:	
		bool IsStackable()
		{
			return (GetFlag() & ITEM_FLAG_STACKABLE) ? true : false; 
		}

		void SetWindow(uint8_t b)
		{
			m_bWindow = b; 
		}

		uint8_t GetWindow()
		{ 
			return m_bWindow;
		}	

		void SetID(uint32_t id)
		{ 
			m_dwID = id;
		}

		uint32_t GetID() const
		{ 
			return m_dwID;
		}

		TItemTable const* GetProto() const 
		{ 
			return m_pProto; 
		}

		const char*	GetName()
		{ 
			return m_pProto ? m_pProto->szLocaleName : NULL; 
		}

		const char*	GetBaseName()
		{ 
			return m_pProto ? m_pProto->szName : NULL; 
		}

		uint8_t GetSize()
		{ 
			return m_pProto ? m_pProto->bSize : 0;	
		}

		void SetFlag(int32_t flag)	
		{ 
			m_lFlag = flag;
		}

		int32_t GetFlag()
		{ 
			return m_lFlag;	
		}

		uint32_t GetAntiFlag()
		{ 
			return m_pProto ? m_pProto->dwAntiFlags : 0; 
		}

		uint32_t GetImmuneFlag() 
		{ 
			return m_pProto ? m_pProto->dwImmuneFlag : 0; 
		}

		void SetVID(uint32_t vid)
		{
			m_dwVID = vid;	
		}

		uint32_t GetVID()
		{
			return m_dwVID;	
		}

		uint32_t GetVnum() const
		{ 
			return m_dwMaskVnum ? m_dwMaskVnum : m_dwVnum;
		}
		
		uint32_t GetDisplayVnum() const 
		{ 
			return GetData() ? GetData() : GetVnum(); 
		}

		uint32_t GetOriginalVnum() const
		{ 
			return m_dwVnum;
		}

		uint8_t GetType() const
		{ 
			return m_pProto ? m_pProto->bType : 0;	
		}

		uint8_t GetSubType() const	
		{ 
			return m_pProto ? m_pProto->bSubType : 0;
		}

		uint8_t GetLimitType(uint32_t idx) const 
		{ 
			return m_pProto ? m_pProto->aLimits[idx].bType : 0;	
		}

		int32_t GetLimitValue(uint32_t idx) const 
		{ 
			return m_pProto ? m_pProto->aLimits[idx].lValue : 0;
		}

		void SetCell(LPCHARACTER ch, uint16_t pos)
		{
			m_pOwner = ch, m_wCell = pos;
		}

		uint16_t GetCell() 
		{ 
			return m_wCell;	
		}

		LPCHARACTER	GetOwner() const
		{
			return m_pOwner;
		}

		bool IsEquipped() const
		{
			return m_bEquipped;
		}

		bool IsExchanging()	
		{ 
			return m_bExchanging;
		}

		const int32_t* GetSockets()
		{ 
			return &m_alSockets[0];
		}

		int32_t GetSocket(int32_t i)
		{ 
			return m_alSockets[i];	
		}

		const TPlayerItemAttribute* GetAttributes()
		{ 
			return m_aAttr;	
		} 

		const TPlayerItemAttribute& GetAttribute(int32_t i)
		{ 
			return m_aAttr[i];	
		}

		uint8_t GetAttributeType(int32_t i)
		{ 
			return m_aAttr[i].bType;
		}

		short GetAttributeValue(int32_t i)
		{ 
			return m_aAttr[i].sValue;
		}	

		uint32_t GetRefinedVnum()
		{
			return m_pProto ? m_pProto->dwRefinedVnum : 0; 
		}

		void SetSkipSave(bool b)
		{ 
			m_bSkipSave = b; 
		}

		bool GetSkipSave()
		{ 
			return m_bSkipSave;
		}

		uint32_t GetLastOwnerPID()
		{ 
			return m_dwLastOwnerPID;
		}

		uint16_t GetRefineSet()
		{ 
			return m_pProto ? m_pProto->wRefineSet : 0;	
		}

		void Lock(bool f) 
		{ 
			m_isLocked = f; 
		}

		bool isLocked() const 
		{ 
			return m_isLocked; 
		}
		
		void SetMaskVnum(uint32_t vnum)	
		{
			m_dwMaskVnum = vnum;
		}

		uint32_t GetMaskVnum()
		{	
			return m_dwMaskVnum; 
		}

		bool IsMaskedItem()	
		{	
			return m_dwMaskVnum != 0;
		}

		void SetSIGVnum(uint32_t dwSIG)
		{
			m_dwSIGVnum = dwSIG;
		}

		uint32_t GetSIGVnum() const
		{
			return m_dwSIGVnum;
		}

		int32_t GetData() const 
		{ 
			return m_iData; 
		}
		
		const bool IsMount()
		{ 
			return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT; 
		}

	private:
		int32_t m_iData;

	protected:
		friend class CInputDB;
		bool OnAfterCreatedItem();

	private:
		TItemTable const* m_pProto;

		uint32_t m_dwVnum;
		LPCHARACTER m_pOwner;
		uint8_t m_bWindow;
		uint32_t m_dwID;
		bool m_bEquipped;
		uint32_t m_dwVID;
		uint16_t m_wCell;
		uint32_t m_dwCount;
		int32_t m_lFlag;
		uint32_t m_dwLastOwnerPID;
		bool m_bExchanging;

		LPEVENT m_pkDestroyEvent;
		LPEVENT m_pkExpireEvent;
		LPEVENT m_pkUniqueExpireEvent;
		LPEVENT m_pkTimerBasedOnWearExpireEvent;
		LPEVENT m_pkRealTimeExpireEvent;
		LPEVENT m_pkAccessorySocketExpireEvent;
		LPEVENT m_pkOwnershipEvent;

		uint32_t m_dwOwnershipPID;
		bool m_bSkipSave;
		bool m_isLocked;
		uint32_t m_dwMaskVnum;
		uint32_t m_dwSIGVnum;

		int32_t m_alSockets[ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute m_aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};

EVENTINFO (item_event_info)
{
	LPITEM item;
	char szOwnerName[CHARACTER_NAME_MAX_LEN];
	item_event_info(): item (0)
	{
		::memset (szOwnerName, 0, CHARACTER_NAME_MAX_LEN);
	}
};

EVENTINFO (item_vid_event_info)
{
	uint32_t item_vid;
	item_vid_event_info(): item_vid (0) {}
};

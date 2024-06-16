#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "desc.h"
#include "sectree_manager.h"
#include "packet.h"
#include "protocol.h"
#include "log.h"
#include "skill.h"
#include "unique_item.h"
#include "marriage.h"
#include "item_addon.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "affect.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#include "../../common/VnumHelper.h"

CItem::CItem(uint32_t dwVnum)
  : m_dwVnum(dwVnum),
	m_bWindow(0),
	m_dwID(0),
	m_bEquipped(false),
	m_dwVID(0),
	m_wCell(0),
	m_dwCount(0),
	m_lFlag(0),
	m_dwLastOwnerPID(0),
	m_bExchanging(false),
	m_pkDestroyEvent(nullptr),
	m_pkExpireEvent(nullptr),
	m_pkUniqueExpireEvent(nullptr),
	m_pkTimerBasedOnWearExpireEvent(nullptr),
	m_pkRealTimeExpireEvent(nullptr),
	m_pkAccessorySocketExpireEvent(nullptr),
	m_pkOwnershipEvent(nullptr),
	m_dwOwnershipPID(0),
	m_bSkipSave(false),
	m_isLocked(false),
	m_dwMaskVnum(0),
	m_dwSIGVnum(0),
	m_alSockets{},
	m_aAttr{}
{
	//Constructor of the body - empty, as all initializations were performed in the initialization list.
}

CItem::~CItem()
{
	Destroy();
}

void CItem::Initialize()
{
	CEntity::Initialize(ENTITY_ITEM);

	m_bWindow = RESERVED_WINDOW;
	m_pOwner = nullptr;
	m_dwID = 0;
	m_bEquipped = false;
	m_dwVID = 0;
	m_wCell = 0;
	m_dwCount = 0;
	m_lFlag = 0;
	m_pProto = nullptr;
	m_bExchanging = false;
	
	std::fill(std::begin(m_alSockets), std::end(m_alSockets), 0);
	std::fill(std::begin(m_aAttr), std::end(m_aAttr), TPlayerItemAttribute());
	
	m_pkDestroyEvent = nullptr;
	m_pkOwnershipEvent = nullptr;
	m_dwOwnershipPID = 0;
	m_pkUniqueExpireEvent = nullptr;
	m_pkTimerBasedOnWearExpireEvent = nullptr;
	m_pkRealTimeExpireEvent = nullptr;
	m_pkAccessorySocketExpireEvent = nullptr;
	m_bSkipSave = false;
	m_dwLastOwnerPID = 0;
}

void CItem::Destroy()
{
	event_cancel(&m_pkDestroyEvent);
	event_cancel(&m_pkOwnershipEvent);
	event_cancel(&m_pkUniqueExpireEvent);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);
	event_cancel(&m_pkRealTimeExpireEvent);
	event_cancel(&m_pkAccessorySocketExpireEvent);

	CEntity::Destroy();

	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
	}
}

EVENTFUNC(itemDestroyEvent)
{
	auto* eventInfo = dynamic_cast<item_event_info*>(event->info);

	if (!eventInfo)
	{
		sys_err("itemDestroyEvent: Failed to cast event info. Null pointer.");
		return 0;
	}

	auto* itemToDestroy = eventInfo->item;

	if (!itemToDestroy)
	{
		sys_err("itemDestroyEvent: Invalid item pointer.");
		return 0;
	}

	if (auto* itemOwner = itemToDestroy->GetOwner())
	{
		sys_err("itemDestroyEvent: Item destruction aborted. Item '%s' has owner '%s'.", itemToDestroy->GetName(), itemOwner->GetName());
	}

	itemToDestroy->SetDestroyEvent(nullptr);
	M2_DESTROY_ITEM(itemToDestroy);

	return 0;
}

void CItem::SetDestroyEvent(LPEVENT pkEvent)
{
	m_pkDestroyEvent = pkEvent;
}

void CItem::StartDestroyEvent(int32_t iSec)
{
	// Preventing multiple executions of the destruction event
	if (m_pkDestroyEvent)
	{
		return;
	}

	// Allocate and initialize event information structure
	auto* info = AllocEventInfo<item_event_info>();
	
	if (!info)
	{
		sys_err("Allocation error in StartDestroyEvent for item ID: %u", GetID());
		return;
	}
	
	info->item = this;

	auto event = event_create(itemDestroyEvent, info, PASSES_PER_SEC(iSec));
	
	if (event)
	{
		SetDestroyEvent(event.get());
	}
	else
	{
		sys_err("Event creation error in StartDestroyEvent for item ID: %u", GetID());
	}
}

void CItem::EncodeInsertPacket(LPENTITY entity)
{
	if (!entity)
	{
		sys_err("CItem::EncodeInsertPacket: Entity pointer is null");
		return;
	}

	auto desc = entity->GetDesc();

	if (!desc)
	{
		return;
	}

	const PIXEL_POSITION &position = GetXYZ();

	TPacketGCItemGroundAdd packetItemGroundAdd
	{
		.bHeader = HEADER_GC_ITEM_GROUND_ADD,
		.x = position.x,
		.y = position.y,
		.z = position.z,
		.dwVnum = GetVnum(),
		.dwVID = m_dwVID,
		.dwItemCount = m_dwCount
	};

	desc->Packet(&packetItemGroundAdd, sizeof(packetItemGroundAdd));

	if (m_pkOwnershipEvent != nullptr)
	{
		auto info = dynamic_cast<item_event_info *>(m_pkOwnershipEvent->info);

		if (!info)
		{
			sys_err("CItem::EncodeInsertPacket: Ownership event info is null for item VID %u", m_dwVID);
			return;
		}

		TPacketGCItemOwnership packetItemOwnership
		{
			.bHeader = HEADER_GC_ITEM_OWNERSHIP,
			.dwVID = m_dwVID
		};

		std::strncpy(packetItemOwnership.szName, info->szOwnerName, sizeof(packetItemOwnership.szName) - 1);
		desc->Packet(&packetItemOwnership, sizeof(TPacketGCItemOwnership));
	}
}

void CItem::EncodeRemovePacket(LPENTITY entity)
{
	if (!entity)
	{
		sys_err("CItem::EncodeRemovePacket: Entity pointer is null");
		return;
	}

	LPDESC desc = entity->GetDesc();

	if (!desc)
	{
		return;
	}

	packet_item_ground_del packetRemoveItem = 
	{
		.bHeader = HEADER_GC_ITEM_GROUND_DEL,
		.dwVID = this->m_dwVID,
	};

	desc->Packet(&packetRemoveItem, sizeof(packetRemoveItem));

	LPCHARACTER character = dynamic_cast<LPCHARACTER>(entity);

	if (character)
	{
		sys_log(2, "Item::EncodeRemovePacket: Item [%s] removed for Character [%s]", GetName(), character->GetName());
	}
	else
	{
		sys_log(1, "Item::EncodeRemovePacket: Item [%s] removed for non-character entity", GetName());
	}
}

void CItem::SetProto(const TItemTable* table)
{
	assert(table != nullptr);
	m_pProto = table;
	SetFlag(m_pProto->dwFlags);
}

void CItem::UsePacketEncode(LPCHARACTER character, LPCHARACTER targetCharacter, struct packet_item_use *usePacket)
{
	if (!usePacket || !character || !targetCharacter)
	{
		sys_log(1, "UsePacketEncode: Null pointer provided for packet, character, or targetCharacter");
		return;
	}

	if (!GetVnum())
	{
		sys_log(1, "UsePacketEncode: Invalid item Vnum for item use packet encoding");
		return;
	}

	*usePacket = 
	{
		.header = HEADER_GC_ITEM_USE,
		.Cell = TItemPos(GetWindow(), m_wCell),
		.ch_vid = character->GetVID(),
		.victim_vid = targetCharacter->GetVID(),
		.vnum = GetVnum()
	};
}

void CItem::AddFlag(int32_t bit)
{
	SET_BIT(m_lFlag, bit);
}

void CItem::RemoveFlag(int32_t bit)
{
	REMOVE_BIT(m_lFlag, bit);
}

void CItem::UpdatePacket()
{
	if (!m_pOwner || !m_pOwner->GetDesc())
	{
		return;
	}

	TPacketGCItemUpdate itemUpdatePacket =
	{
		.header = HEADER_GC_ITEM_UPDATE,
		.Cell = TItemPos(GetWindow(), m_wCell),
		.count = static_cast<uint8_t>(m_dwCount)
	};

    std::copy(std::begin(m_alSockets), std::begin(m_alSockets) + ITEM_SOCKET_MAX_NUM, std::begin(itemUpdatePacket.alSockets));
	
	if (auto* attributes = GetAttributes(); attributes)
	{
		std::copy(attributes, attributes + sizeof(itemUpdatePacket.aAttr) / sizeof(itemUpdatePacket.aAttr[0]), itemUpdatePacket.aAttr);
	}

	sys_log(2, "UpdatePacket %s -> %s", GetName(), m_pOwner->GetName());
	m_pOwner->GetDesc()->Packet(&itemUpdatePacket, sizeof(itemUpdatePacket));
}

bool CItem::SetCount(uint32_t count)
{
	const uint32_t MAX_ELK_COUNT = static_cast<uint32_t>(INT_MAX);
	const uint32_t MAX_ITEM_COUNT = ITEM_MAX_COUNT;

	m_dwCount = (GetType() == ITEM_ELK) ? std::min(count, MAX_ELK_COUNT) : std::min(count, MAX_ITEM_COUNT);

	if (count == 0 && m_pOwner)
	{
		auto itemSubType = GetSubType();
		auto itemVnum = GetVnum();
		
		if (itemSubType == USE_ABILITY_UP || itemSubType == USE_POTION || itemVnum == 70020)
		{
			auto owner = GetOwner();
			auto cellIndex = GetCell();

			RemoveFromCharacter();

			if (!IsDragonSoul())
			{
				auto foundItem = owner->FindSpecifyItem(itemVnum);

				if (foundItem)
				{
					owner->ChainQuickslotItem(foundItem, QUICKSLOT_TYPE_ITEM, cellIndex);
				}
				else
				{
					owner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, cellIndex, 255);
				}
			}

			M2_DESTROY_ITEM(this);
		}
		else
		{
			if (!IsDragonSoul())
			{
				m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, m_wCell, 255);
			}
			
			M2_DESTROY_ITEM(RemoveFromCharacter());
		}

		return false;
	}

	UpdatePacket();
	Save();
	
	return true;
}

uint32_t CItem::GetCount() const
{
	return (GetType() == ITEM_ELK) ? std::min(m_dwCount, static_cast<uint32_t>(INT_MAX)) : std::min(m_dwCount, static_cast<uint32_t>(200));
}

bool CItem::AddToCharacter(LPCHARACTER character, TItemPos cellPosition)
{
	assert(GetSectree() == nullptr);
	assert(m_pOwner == nullptr);

	uint16_t position = cellPosition.cell;
	uint8_t windowType = cellPosition.window_type;

	if (INVENTORY == windowType)
	{
		if (m_wCell >= INVENTORY_MAX_NUM && BELT_INVENTORY_SLOT_START > m_wCell)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, character->GetName(), m_wCell);
			return false;
		}
	}
	else if (DRAGON_SOUL_INVENTORY == windowType)
	{
		if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, character->GetName(), m_wCell);
			return false;
		}
	}

	if (character->GetDesc())
	{
		m_dwLastOwnerPID = character->GetPlayerID();
	}

	event_cancel(&m_pkDestroyEvent);

	character->SetItem(TItemPos(windowType, position), this);
	m_pOwner = character;

	Save();
	return true;
}

LPITEM CItem::RemoveFromCharacter()
{
	if (!m_pOwner)
	{
		sys_err("Item::RemoveFromCharacter owner null");
		return this;
	}

	LPCHARACTER owner = m_pOwner;

	if (m_bEquipped)
	{
		Unequip();
		SetWindow(RESERVED_WINDOW);
		Save();
		return this;
	}
	else
	{
		if (GetWindow() != SAFEBOX && GetWindow() != MALL)
		{
			if (IsDragonSoul())
			{
				assert(m_wCell < DRAGON_SOUL_INVENTORY_MAX_NUM);
				owner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
			}		
			else
			{
				TItemPos inventoryCell(INVENTORY, m_wCell);

				assert(inventoryCell.IsDefaultInventoryPosition() || inventoryCell.IsBeltInventoryPosition());
				owner->SetItem(inventoryCell, nullptr);
			}
		}

		m_pOwner = nullptr;
		m_wCell = 0;

		SetWindow(RESERVED_WINDOW);
		Save();
		return this;
	}
}

bool CItem::AddToGround(int32_t mapIndex, const PIXEL_POSITION& position, bool skipOwnerCheck)
{
	if (0 == mapIndex)
	{
		sys_err("wrong map index argument: %ld", mapIndex);
		return false;
	}

	if (GetSectree())
	{
		sys_err("sectree already assigned");
		return false;
	}

	if (!skipOwnerCheck && m_pOwner)
	{
		sys_err("owner pointer not null");
		return false;
	}

	LPSECTREE tree = SECTREE_MANAGER::instance().Get(mapIndex, position.x, position.y);

	if (!tree)
	{
		sys_err("cannot find sectree by %dx%d", position.x, position.y);
		return false;
	}

	SetWindow(GROUND);
	SetXYZ(position.x, position.y, position.z);
	
	tree->InsertEntity(this);
	
	UpdateSectree();
	Save();
	
	return true;
}

LPITEM CItem::RemoveFromGround()
{
	if (GetSectree())
	{
		SetOwnership(nullptr);
		GetSectree()->RemoveEntity(this);
		ViewCleanup();
		Save();
	}

	return this;
}

bool CItem::DistanceValid(LPCHARACTER character)
{
	if (!GetSectree())
	{
		return false;
	}
	
	int32_t distance = DISTANCE_APPROX(GetX() - character->GetX(), GetY() - character->GetY());
	constexpr int32_t MAX_DISTANCE = 300;

	return distance <= MAX_DISTANCE;
}

bool CItem::CanCharacterEquipItem(LPCHARACTER character)
{
	uint32_t antiFlag = GetAntiFlag();

	switch (character->GetJob())
	{
		case JOB_WARRIOR:
		{
			return !(antiFlag & ITEM_ANTIFLAG_WARRIOR);
		}

		case JOB_ASSASSIN:
		{
			return !(antiFlag & ITEM_ANTIFLAG_ASSASSIN);
		}

		case JOB_SHAMAN:
		{
			return !(antiFlag & ITEM_ANTIFLAG_SHAMAN);
		}

		case JOB_SURA:
		{
			return !(antiFlag & ITEM_ANTIFLAG_SURA);
		}

		default:
		{
			return true;
		}
	}

	return true;
}

int32_t CItem::FindEquipCell(LPCHARACTER character, int32_t iCandidateCell)
{
	auto itemType = static_cast<EItemTypes>(GetType());

	if (!IsEquippableItemType(itemType))
	{
		return -1;
	}

	if (IsItemTypeDragonSoul(itemType))
	{
		return CalculateDragonSoulCell(iCandidateCell);
	}
	else if (itemType == ITEM_COSTUME)
	{
		return CalculateCostumeEquipCell();
	}
	else if (itemType == ITEM_RING)
	{
		return CalculateRingEquipCell(character);
	}
	else if (itemType == ITEM_BELT)
	{
		return WEAR_BELT;
	}

	int32_t wearType = CalculateEquipCellByWearFlag(GetWearFlag(), character);
	
	if (wearType != -1)
	{
		return wearType;
	}

	return CalculateAbilityEquipCell(character);
}

bool CItem::IsEquippableItemType(EItemTypes itemType)
{
	return !(0 == GetWearFlag() || ITEM_TOTEM == itemType) || (ITEM_COSTUME == itemType || ITEM_DS == itemType || ITEM_SPECIAL_DS == itemType || ITEM_RING == itemType || ITEM_BELT == itemType);
}

bool CItem::IsItemTypeDragonSoul(EItemTypes itemType)
{
	return itemType == ITEM_DS || itemType == ITEM_SPECIAL_DS;
}

int32_t CItem::CalculateDragonSoulCell(int32_t iCandidateCell)
{
	if (iCandidateCell < 0)
	{
		return WEAR_MAX_NUM + GetSubType();
	}
	else
	{
		for (int32_t i = 0; i < DRAGON_SOUL_DECK_MAX_NUM; i++)
		{
			if (WEAR_MAX_NUM + i * DS_SLOT_MAX + GetSubType() == iCandidateCell)
			{
				return iCandidateCell;
			}
		}
		
		return -1;
	}
}

int32_t CItem::CalculateCostumeEquipCell()
{
	switch (GetSubType())
	{
		case COSTUME_BODY: return WEAR_COSTUME_BODY;
		case COSTUME_HAIR: return WEAR_COSTUME_HAIR;
		case COSTUME_MOUNT: return WEAR_COSTUME_MOUNT;
		default: return -1;
	}
}

int32_t CItem::CalculateRingEquipCell(LPCHARACTER character)
{
	return character->GetWear(WEAR_RING1) ? WEAR_RING2 : WEAR_RING1;
}

int32_t CItem::CalculateEquipCellByWearFlag(int32_t wearFlag, LPCHARACTER character)
{
	using WearMapping = std::pair<int32_t, int32_t>;

	constexpr std::array<WearMapping, 10> wearMapping = 
	{{
		{WEARABLE_BODY, WEAR_BODY},
		{WEARABLE_HEAD, WEAR_HEAD},
		{WEARABLE_FOOTS, WEAR_FOOTS},
		{WEARABLE_WRIST, WEAR_WRIST},
		{WEARABLE_WEAPON, WEAR_WEAPON},
		{WEARABLE_SHIELD, WEAR_SHIELD},
		{WEARABLE_NECK, WEAR_NECK},
		{WEARABLE_EAR, WEAR_EAR},
		{WEARABLE_ARROW, WEAR_ARROW},
		{WEARABLE_UNIQUE, WEAR_UNIQUE1}
	}};

	for (const auto& [flag, wearType] : wearMapping) 
	{
		if (wearFlag & flag) 
		{
			if (flag == WEARABLE_UNIQUE)
			{
				return (character->GetWear(WEAR_UNIQUE1) ? WEAR_UNIQUE2 : WEAR_UNIQUE1);
			}

			return wearType;
		}
	}

	return -1;
}

int32_t CItem::CalculateAbilityEquipCell(LPCHARACTER character)
{
	if (GetWearFlag() & WEARABLE_ABILITY)
	{
		for (int32_t abilitySlot = WEAR_ABILITY1; abilitySlot <= WEAR_ABILITY8; abilitySlot++)
		{
			if (!character->GetWear(abilitySlot))
			{
				return abilitySlot;
			}
		}
	}
	
	return -1;
}

void CItem::ModifyPoints(bool bAdd, LPCHARACTER character)
{
	if (!character) 
	{
		character = m_pOwner;
		
		if (!character)
		{
			return;
		}
	}

	int32_t accessoryGrade = (IsAccessoryForSocket()) ? MIN(GetAccessorySocketGrade(), ITEM_ACCESSORY_SOCKET_MAX_NUM) : 0;

	if (false == IsAccessoryForSocket() && (m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR)) 
	{
		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) 
		{
			uint32_t dwVnum = GetSocket(i);
			
			if (dwVnum <= 2) 
			{
				continue;
			}

			TItemTable *p = ITEM_MANAGER::instance().GetTable(dwVnum);
			
			if (!p) 
			{
				continue;
			}

			if (ITEM_METIN == p->bType) 
			{
				for (int32_t j = 0; j < ITEM_APPLY_MAX_NUM; ++j) 
				{
					if (p->aApplies[j].bType == APPLY_NONE) 
					{
						continue;
					}

					int32_t applyValue = (bAdd ? p->aApplies[j].lValue : (p->aApplies[j].bType == APPLY_SKILL ? p->aApplies[j].lValue ^ 0x00800000 : -p->aApplies[j].lValue));
					character->ApplyPoint(p->aApplies[j].bType, applyValue);
				}
			}
		}
	}

	for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i) 
	{
		if (m_pProto->aApplies[i].bType == APPLY_NONE) 
		{
			continue;
		}
		
		if (IsMount())
		{
			continue;
		}

		int32_t value = m_pProto->aApplies[i].lValue;
		
		if (0 != accessoryGrade && m_pProto->aApplies[i].bType != APPLY_SKILL) 
		{
			value += MAX(accessoryGrade, value * aiAccessorySocketEffectivePct[accessoryGrade] / 100);
		}

		int32_t applyValue = (bAdd ? value : (m_pProto->aApplies[i].bType == APPLY_SKILL ? value ^ 0x00800000 : -value));
		character->ApplyPoint(m_pProto->aApplies[i].bType, applyValue);
	}

	if (!CItemVnumHelper::IsSpecialItem(GetVnum())) 
	{
		for (int32_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i) 
		{
			if (GetAttributeType(i)) 
			{
				const TPlayerItemAttribute& ia = GetAttribute(i);
				int32_t applyValue = (bAdd ? ia.sValue : (ia.bType == APPLY_SKILL ? ia.sValue ^ 0x00800000 : -ia.sValue));
				character->ApplyPoint(ia.bType, applyValue);
			}
		}
	}

	switch (m_pProto->bType)
	{
		case ITEM_PICK:
		case ITEM_ROD:
		case ITEM_WEAPON:
		{
			if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
			{
				uint32_t partValue = bAdd ? GetVnum() : character->GetOriginalPart(PART_WEAPON);
				character->SetPart(PART_WEAPON, partValue);
			}
		}
		break;

		case ITEM_ARMOR:
		{
			if (0 != character->GetWear(WEAR_COSTUME_BODY))
			{
				break;
			}

			if (GetSubType() == ARMOR_BODY)
			{
				if (bAdd)
				{
					if (GetProto()->bSubType == ARMOR_BODY)
					{
						character->SetPart(PART_MAIN, GetVnum());
					}
				}
				else
				{
					if (GetProto()->bSubType == ARMOR_BODY)
					{
						character->SetPart(PART_MAIN, character->GetOriginalPart(PART_MAIN));
					}
				}
			}
		}
		break;

		case ITEM_COSTUME:
		{
			uint32_t toSetValue = this->GetVnum();
			EParts targetPart = PART_MAX_NUM;

			if (GetSubType() == COSTUME_BODY)
			{
				targetPart = PART_MAIN;
				if (!bAdd)
				{
					const CItem* armor = m_pOwner->GetWear(WEAR_BODY);
					toSetValue = (nullptr != armor) ? armor->GetVnum() : m_pOwner->GetOriginalPart(PART_MAIN);
				}

			}
			else if (GetSubType() == COSTUME_HAIR)
			{
				targetPart = PART_HAIR;
				toSetValue = bAdd ? this->GetValue(3) : 0;
			}

			if (PART_MAX_NUM != targetPart)
			{
				m_pOwner->SetPart((uint8_t)targetPart, toSetValue);
				m_pOwner->UpdatePacket();
			}
		}
		break;
		
		case ITEM_UNIQUE:
		{
			uint32_t sigVnum = GetSIGVnum();
			
			if (sigVnum != 0)
			{
				const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(sigVnum);
				
				if (pItemGroup)
				{
					uint32_t dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());
					const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(dwAttrVnum);
					
					if (pAttrGroup)
					{
						int32_t applyModifier = bAdd ? 1 : -1;
						
						for (const auto& attr : pAttrGroup->m_vecAttrs)
						{
							character->ApplyPoint(attr.apply_type, applyModifier * attr.apply_value);
						}
					}
				}
			}
		}
		break;
	}
}

bool CItem::IsEquipable() const
{
	switch (this->GetType())
	{
		case ITEM_COSTUME:
		case ITEM_ARMOR:
		case ITEM_WEAPON:
		case ITEM_ROD:
		case ITEM_PICK:
		case ITEM_UNIQUE:
		case ITEM_DS:
		case ITEM_SPECIAL_DS:
		case ITEM_RING:
		case ITEM_BELT:
		{
			return true;
		}
	}

	return false;
}

bool CItem::EquipTo(LPCHARACTER character, uint8_t bWearCell)
{
	if (!character || (IsDragonSoul() && (bWearCell < WEAR_MAX_NUM || bWearCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)) || (!IsDragonSoul() && bWearCell >= WEAR_MAX_NUM))
	{
		sys_err("EquipTo: Invalid input (character: %p, cell: %d, dragon soul: %s)", character, bWearCell, IsDragonSoul() ? "yes" : "no");
		return false;
	}

	if (character->GetWear(bWearCell))
	{
		sys_err("EquipTo: Item already exists (Vnum: %d, Name: %s, Cell: %d)", GetOriginalVnum(), GetName(), bWearCell);
		return false;
	}
	
	if (GetOwner())
	{
		RemoveFromCharacter();
	}

	character->SetWear(bWearCell, this);

	m_pOwner = character;
	m_bEquipped = true;
	m_wCell	= INVENTORY_MAX_NUM + bWearCell;

	uint32_t dwImmuneFlag = 0;
	
	for (int32_t i = 0; i < WEAR_MAX_NUM; ++i)
	{
		const CItem* pItem = character->GetWear(i);
		
		if (pItem)
		{
			dwImmuneFlag |= pItem->m_pProto->dwImmuneFlag;
		}
	}

	character->SetImmuneFlag(dwImmuneFlag);

	if (IsDragonSoul())
	{
		DSManager::instance().ActivateDragonSoul(this);
	}
	else
	{
		ModifyPoints(true);	
		StartUniqueExpireEvent();
		
		if (GetProto()->cLimitTimerBasedOnWearIndex != -1)
		{
			StartTimerBasedOnWearExpireEvent();
		}

		StartAccessorySocketExpireEvent();
	}

	character->BuffOnAttr_AddBuffsFromItem(this);
	
	m_pOwner->ComputeBattlePoints();
	
	m_pOwner->UpdatePacket();
	Save();

	return (true);
}

bool CItem::Unequip()
{
	if (!m_pOwner || GetCell() < INVENTORY_MAX_NUM)
	{
		sys_err("%s %u m_pOwner %p, GetCell %d", GetName(), GetID(), get_pointer(m_pOwner), GetCell());
		return false;
	}

	if (this != m_pOwner->GetWear(GetCell() - INVENTORY_MAX_NUM))
	{
		sys_err("m_pOwner->GetWear() != this");
		return false;
	}

	if (IsRideItem())
	{
		ClearMountAttributeAndAffect();
	}

	if (IsDragonSoul())
	{
		DSManager::instance().DeactivateDragonSoul(this);
	}
	else
	{
		ModifyPoints(false);
	}

	StopUniqueExpireEvent();

	if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
	{
		StopTimerBasedOnWearExpireEvent();
	}

	StopAccessorySocketExpireEvent();

	m_pOwner->BuffOnAttr_RemoveBuffsFromItem(this);
	m_pOwner->SetWear(GetCell() - INVENTORY_MAX_NUM, nullptr);

	uint32_t dwImmuneFlag = 0;

	for (int32_t i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (m_pOwner->GetWear(i))
		{
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
	m_pOwner->ComputeBattlePoints();
	m_pOwner->UpdatePacket();

	m_pOwner = nullptr;
	m_wCell = 0;
	m_bEquipped = false;

	return true;
}

int32_t CItem::GetValue(uint32_t idx)
{
	assert(idx < ITEM_VALUES_MAX_NUM);
	return GetProto()->alValues[idx];
}

void CItem::SetExchanging(bool bOn)
{
	m_bExchanging = bOn;
}

void CItem::Save()
{
	if (!m_bSkipSave)
	{
		ITEM_MANAGER::instance().DelayedSave(this);
	}
}

bool CItem::CreateSocket(uint8_t bSlot, uint8_t bGold)
{
	assert(bSlot < ITEM_SOCKET_MAX_NUM);

	if (m_alSockets[bSlot] != 0)
	{
		sys_err("Item::CreateSocket : socket already exist %s %d", GetName(), bSlot);
		return false;
	}

	m_alSockets[bSlot] = bGold ? 2 : 1;

	UpdatePacket();
	Save();
	
	return true;
}

void CItem::SetSockets(const int32_t* c_al)
{
	assert(c_al != nullptr);
	std::copy(c_al, c_al + ITEM_SOCKET_MAX_NUM, m_alSockets);
	Save();
}

void CItem::SetSocket(int32_t i, int32_t v, bool bLog)
{
	assert(i >= 0 && i < ITEM_SOCKET_MAX_NUM);
	m_alSockets[i] = v;
	UpdatePacket();
	Save();

	if (bLog)
	{
		LogManager::instance().ItemLog(GetID(), i, v, 0, "SET_SOCKET", "", "", GetOriginalVnum());
	}
}

int32_t CItem::GetGold()
{
	if (IS_SET(GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		return (GetProto()->dwGold == 0) ? GetCount() : GetCount() / GetProto()->dwGold;
	}
	
	return GetProto()->dwGold;
}

int32_t CItem::GetShopBuyPrice()
{
	return GetProto()->dwShopBuyPrice;
}

bool CItem::IsOwnership(LPCHARACTER ch)
{
	if (!m_pkOwnershipEvent)
	{
		return true;
	}

	return m_dwOwnershipPID == ch->GetPlayerID();
}

EVENTFUNC(ownership_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (!info)
	{
		sys_err("ownership_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;

	if (!pkItem)
	{
		sys_err("ownership_event> <Factor> Item pointer is null");
		return 0;
	}

	pkItem->SetOwnershipEvent(nullptr);

	TPacketGCItemOwnership packet
	{ 
		HEADER_GC_ITEM_OWNERSHIP, 
		pkItem->GetVID(), 
		{'\0'}
	};

	pkItem->PacketAround(&packet, sizeof(packet));
	return 0;
}

void CItem::SetOwnershipEvent(LPEVENT pkEvent)
{
	m_pkOwnershipEvent = pkEvent;
}

void CItem::SetOwnership(LPCHARACTER character, int32_t iSec)
{
	if (!character)
	{
		if (m_pkOwnershipEvent)
		{
			event_cancel(&m_pkOwnershipEvent);
			m_dwOwnershipPID = 0;

			TPacketGCItemOwnership packet
			{
				HEADER_GC_ITEM_OWNERSHIP,
				m_dwVID, 
				{'\0'} 
			};
			
			PacketAround(&packet, sizeof(packet));
		}
		return;
	}

	if (m_pkOwnershipEvent)
	{
		return;
	}

	iSec = (LC_IsEurope() && iSec <= 10) ? 30 : iSec;

	m_dwOwnershipPID = character->GetPlayerID();

	auto* info = AllocEventInfo<item_event_info>();
	strlcpy(info->szOwnerName, character->GetName(), sizeof(info->szOwnerName));
	info->item = this;

	SetOwnershipEvent(event_create(ownership_event, info, PASSES_PER_SEC(iSec)));

	TPacketGCItemOwnership pack{ HEADER_GC_ITEM_OWNERSHIP, m_dwVID };
	strlcpy(pack.szName, character->GetName(), sizeof(pack.szName));

	PacketAround(&pack, sizeof(pack));
}

int32_t CItem::GetSocketCount()
{
	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		if (GetSocket(i) == 0)
		{
			return i;
		}
	}
	
	return ITEM_SOCKET_MAX_NUM;
}

bool CItem::AddSocket()
{
	int32_t count = GetSocketCount();
	
	if (count == ITEM_SOCKET_MAX_NUM)
	{
		return false;
	}
	
	m_alSockets[count] = 1;
	return true;
}

void CItem::AlterToSocketItem(int32_t iSocketCount)
{
	iSocketCount = (iSocketCount > ITEM_SOCKET_MAX_NUM) ? ITEM_SOCKET_MAX_NUM : iSocketCount;
	
	for (int32_t i = 0; i < iSocketCount; ++i)
	{
		SetSocket(i, 1);
	}
}

void CItem::AlterToMagicItem()
{
	if (GetAttributeSetIndex() < 0)
	{
		return;
	}

	struct MagicItemChances 
	{
		int32_t secondPct;
		int32_t thirdPct;
	};

	static const std::unordered_map<int32_t, MagicItemChances> itemChances = 
	{
		{ ITEM_WEAPON, {20, 5} },
		{ ITEM_ARMOR, {10, GetSubType() == ARMOR_BODY ? 2 : 1} },
		{ ITEM_COSTUME, {10, GetSubType() == ARMOR_BODY ? 2 : 1} }
	};

	auto it = itemChances.find(GetType());
	
	if (it == itemChances.end())
	{
		return;
	}

	PutAttribute(aiItemMagicAttributePercentHigh);

	if (number(1, 100) <= it->second.secondPct)
	{
		PutAttribute(aiItemMagicAttributePercentLow);
	}

	if (number(1, 100) <= it->second.thirdPct)
	{
		PutAttribute(aiItemMagicAttributePercentLow);
	}
}

uint32_t CItem::GetRefineFromVnum()
{
	return ITEM_MANAGER::instance().GetRefineFromVnum(GetVnum());
}

int32_t CItem::GetRefineLevel()
{
	const char* baseName = GetBaseName();
	char* plusSignPosBase = const_cast<char*>(strrchr(baseName, '+'));

	if (!plusSignPosBase)
	{
		return 0;
	}

	int32_t refineLevelBase = 0;
	str_to_number(refineLevelBase, plusSignPosBase + 1);

	const char* localeName = GetName();
	char* plusSignPosLocale = const_cast<char*>(strrchr(localeName, '+'));

	if (plusSignPosLocale)
	{
		int32_t refineLevelLocale = 0;
		str_to_number(refineLevelLocale, plusSignPosLocale + 1);
		
		if (refineLevelLocale != refineLevelBase)
		{
			sys_err("Discrepancy in refine levels: BaseName [%d] vs LocaleName [%d]", refineLevelBase, refineLevelLocale);
		}
	}

	return refineLevelBase;
}

bool CItem::IsPolymorphItem()
{
	return GetType() == ITEM_POLYMORPH;
}

EVENTFUNC(unique_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (!info)
	{
		sys_err("unique_expire_event> Null pointer in event info");
		return 0;
	}

	LPITEM pkItem = info->item;

	if (!pkItem)
	{
		sys_err("unique_expire_event> Null pointer in pkItem");
		return 0;
	}
	
	const int32_t remainTime = pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME);

	if (pkItem->GetValue(2) == 0)
	{
		if (remainTime <= 1)
		{
			sys_log(0, "UNIQUE_ITEM: expire %s %u", pkItem->GetName(), pkItem->GetID());
			pkItem->SetUniqueExpireEvent(nullptr);
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			pkItem->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, remainTime - 1);
			return PASSES_PER_SEC(60);
		}
	}
	else
	{
		time_t cur = get_global_time();

		if (remainTime <= cur)
		{
			pkItem->SetUniqueExpireEvent(nullptr);
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
		}
		else
		{
			int32_t nextCheck = std::min(600, remainTime - cur);
			return PASSES_PER_SEC(nextCheck);
		}
	}

	return 0;
}

EVENTFUNC(timer_based_on_wear_expire_event)
{
	if (!event || !event->info) 
	{
		sys_err("expire_event <Factor> Null pointer");
		return 0;
	}

	auto* info = dynamic_cast<item_event_info*>(event->info);
	
	if (!info || !(info->item))
	{
		sys_err("expire_event <Info> Null pointer or item not found");
		return 0;
	}
	
	LPITEM pkItem = info->item;
	int32_t remain_time = pkItem->GetSocket(ITEM_SOCKET_REMAIN_SEC) - processing_time / passes_per_sec;

	if (remain_time <= 0)
	{
		sys_log(0, "ITEM EXPIRED: %s %u", pkItem->GetName(), pkItem->GetID());
		pkItem->SetTimerBasedOnWearExpireEvent(nullptr);
		pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, 0);

		if (pkItem->IsDragonSoul())
		{
			DSManager::instance().DeactivateDragonSoul (pkItem);
		}
		else
		{
			ITEM_MANAGER::instance().RemoveItem(pkItem, "TIMER_BASED_ON_WEAR_EXPIRE");
		}
		
		return 0;
	}
	
	pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	return PASSES_PER_SEC(std::max(1, std::min(60, remain_time)));
}

void CItem::SetUniqueExpireEvent(LPEVENT pkEvent)
{
	m_pkUniqueExpireEvent = pkEvent;
}

void CItem::SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent)
{
	m_pkTimerBasedOnWearExpireEvent = pkEvent;
}

EVENTFUNC(real_time_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);

	if (!info)
	{
		return 0;
	}

	const LPITEM item = ITEM_MANAGER::instance().FindByVID(info->item_vid);

	if (!item)
	{
		return 0;
	}

	const time_t current = get_global_time();

	if (current > item->GetSocket(0))
	{
		if (item->IsNewMountItem() && item->GetSocket(2) != 0)
		{
			item->ClearMountAttributeAndAffect();
		}

		ITEM_MANAGER::instance().RemoveItem(item, "REAL_TIME_EXPIRE");
		return 0;
	}

	return PASSES_PER_SEC(1);
}

void CItem::StartRealTimeExpireEvent()
{
	if (m_pkRealTimeExpireEvent)
	{
		return;
	}
	
	const TItemLimit* limits = GetProto()->aLimits;

	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (LIMIT_REAL_TIME == limits[i].bType || LIMIT_REAL_TIME_START_FIRST_USE == limits[i].bType)
		{
			item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
			info->item_vid = GetVID();

			m_pkRealTimeExpireEvent = event_create(real_time_expire_event, info, PASSES_PER_SEC(1));
			sys_log(0, "REAL_TIME_EXPIRE: StartRealTimeExpireEvent");
			return;
		}
	}
}

bool CItem::IsRealTimeItem() const
{
	const TItemTable* proto = GetProto();

	if (!proto)
	{
		return false;
	}

	for (int32_t limitIndex = 0; limitIndex < ITEM_LIMIT_MAX_NUM; ++limitIndex)
	{
		if (LIMIT_REAL_TIME == proto->aLimits[limitIndex].bType)
		{
			return true;
		}
	}

	return false;
}

void CItem::StartUniqueExpireEvent()
{
	if (GetType() != ITEM_UNIQUE || m_pkUniqueExpireEvent || IsRealTimeItem())
	{
		return;
	}

	int32_t secondsUntilExpire = GetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME);
	secondsUntilExpire = (secondsUntilExpire == 0) ? 60 : std::min(secondsUntilExpire, 60);
	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, 0);

	item_event_info* eventInfo = AllocEventInfo<item_event_info>();
	eventInfo->item = this;
	SetUniqueExpireEvent(event_create(unique_expire_event, eventInfo, PASSES_PER_SEC(secondsUntilExpire)));

	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE && m_pOwner)
	{
		m_pOwner->ShowAlignment(false);
	}
}

void CItem::StartTimerBasedOnWearExpireEvent()
{
    if (m_pkTimerBasedOnWearExpireEvent || IsRealTimeItem() || GetProto()->cLimitTimerBasedOnWearIndex == -1)
	{
        return;
	}

    int32_t wearTimer = GetSocket(0);
    wearTimer = (wearTimer != 0) ? (wearTimer % 60 == 0 ? 60 : wearTimer % 60) : 60;

    item_event_info* eventInfo = AllocEventInfo<item_event_info>();
    eventInfo->item = this;
    SetTimerBasedOnWearExpireEvent(event_create(timer_based_on_wear_expire_event, eventInfo, PASSES_PER_SEC(wearTimer)));
}

void CItem::StopUniqueExpireEvent()
{
	if (!m_pkUniqueExpireEvent || GetValue(2) != 0)
	{
		return;
	}

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, event_time(m_pkUniqueExpireEvent) / passes_per_sec);
	event_cancel(&m_pkUniqueExpireEvent);

	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE && m_pOwner)
	{
		m_pOwner->ShowAlignment(true);
	}

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::StopTimerBasedOnWearExpireEvent()
{
	if (!m_pkTimerBasedOnWearExpireEvent)
	{
		return;
	}

	int32_t remainingTime = GetSocket(ITEM_SOCKET_REMAIN_SEC) - event_processing_time(m_pkTimerBasedOnWearExpireEvent) / passes_per_sec;
	SetSocket(ITEM_SOCKET_REMAIN_SEC, remainingTime);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::ApplyAddon(int32_t iAddonType)
{
	CItemAddonManager::instance().ApplyAddonTo(iAddonType, this);
}

int32_t CItem::GetSpecialGroup() const
{ 
	return ITEM_MANAGER::instance().GetSpecialGroupFromItem(GetVnum()); 
}

bool CItem::IsAccessoryForSocket()
{
	return (m_pProto->bType == ITEM_ARMOR && (m_pProto->bSubType == ARMOR_WRIST || m_pProto->bSubType == ARMOR_NECK || m_pProto->bSubType == ARMOR_EAR)) || (m_pProto->bType == ITEM_BELT);			
}

void CItem::SetAccessorySocketGrade(int32_t iGrade) 
{ 
	SetSocket(0, MINMAX(0, iGrade, GetAccessorySocketMaxGrade())); 
	int32_t iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];
	SetAccessorySocketDownGradeTime(iDownTime);
}

void CItem::SetAccessorySocketMaxGrade(int32_t iMaxGrade) 
{ 
	SetSocket(1, MINMAX(0, iMaxGrade, ITEM_ACCESSORY_SOCKET_MAX_NUM)); 
}

void CItem::SetAccessorySocketDownGradeTime(uint32_t time) 
{ 
	SetSocket(2, time); 

	if (test_server && GetOwner())
	{
		GetOwner()->ChatPacket (CHAT_TYPE_INFO, "[LS;711;%s;%d]", GetName(), time);
	}
}

EVENTFUNC(accessory_socket_expire_event)
{
	auto* info = dynamic_cast<item_vid_event_info*>(event->info);

	if (!info)
	{
		sys_err("accessory_socket_expire_event: Null pointer in event info");
		return 0;
	}

	auto* item = ITEM_MANAGER::instance().FindByVID(info->item_vid);
	
	if (!item)
	{
		sys_err("accessory_socket_expire_event: Item not found");
		return 0;
	}	
	
	int32_t remainingTime = item->GetAccessorySocketDownGradeTime() - 60;
	
	if (remainingTime <= 1)
	{
		item->SetAccessorySocketExpireEvent(nullptr);
		item->AccessorySocketDegrade();
		return 0;
	}

	item->SetAccessorySocketDownGradeTime(remainingTime);
	return PASSES_PER_SEC(std::min(remainingTime, 60));
}

void CItem::StartAccessorySocketExpireEvent()
{
    if (!IsAccessoryForSocket() || m_pkAccessorySocketExpireEvent || GetAccessorySocketMaxGrade() == 0 || GetAccessorySocketGrade() == 0)
	{
        return;
	}

    int32_t timeUntilExpire = std::max(GetAccessorySocketDownGradeTime(), 5);
    timeUntilExpire = std::min(timeUntilExpire, 60);

    auto* eventInfo = AllocEventInfo<item_vid_event_info>();
    eventInfo->item_vid = GetVID();

    SetAccessorySocketExpireEvent(event_create(accessory_socket_expire_event, eventInfo, PASSES_PER_SEC(timeUntilExpire)));
}

void CItem::StopAccessorySocketExpireEvent()
{
	if (!m_pkAccessorySocketExpireEvent || !IsAccessoryForSocket())
	{
		return;
	}

	int32_t adjustedTime = GetAccessorySocketDownGradeTime() - (60 - event_time(m_pkAccessorySocketExpireEvent) / passes_per_sec);
	event_cancel(&m_pkAccessorySocketExpireEvent);

	if (adjustedTime <= 1)
	{
		AccessorySocketDegrade();
	}
	else
	{
		SetAccessorySocketDownGradeTime(adjustedTime);
	}
}
		
bool CItem::IsRideItem()
{
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType())
	{
		return true;
	}

	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType())
	{
		return true;
	}
	
	if (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType())
	{
		return true;
	}

	return false;
}

bool CItem::IsRamadanRing()
{
	return GetVnum() == UNIQUE_ITEM_RAMADAN_RING;
}

void CItem::ClearMountAttributeAndAffect()
{
	LPCHARACTER pOwner = GetOwner();

	if (!pOwner)
	{
		sys_err("ClearMountAttributeAndAffect called without owner");
		return;
	}

	pOwner->RemoveAffect(AFFECT_MOUNT);
	pOwner->RemoveAffect(AFFECT_MOUNT_BONUS);

	pOwner->MountVnum(0);

	pOwner->PointChange(POINT_ST, 0);
	pOwner->PointChange(POINT_DX, 0);
	pOwner->PointChange(POINT_HT, 0);
	pOwner->PointChange(POINT_IQ, 0);
}

bool CItem::IsNewMountItem()
{
	return ((ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType() && IS_SET(GetFlag(), ITEM_FLAG_QUEST_USE))
			   || (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType() && IS_SET(GetFlag(), ITEM_FLAG_QUEST_USE))
			   || (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType()));
}

void CItem::SetAccessorySocketExpireEvent(LPEVENT pkEvent)
{
	m_pkAccessorySocketExpireEvent = pkEvent;
}

void CItem::AccessorySocketDegrade()
{
	if (GetAccessorySocketGrade() > 0)
	{
		LPCHARACTER ch = GetOwner();

		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;710;%s]", GetName());
		}

		ModifyPoints(false);
		SetAccessorySocketGrade(GetAccessorySocketGrade()-1);
		ModifyPoints(true);

		int32_t iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

		if (test_server)
		{
			iDownTime /= 60;
		}

		SetAccessorySocketDownGradeTime(iDownTime);

		if (iDownTime)
		{
			StartAccessorySocketExpireEvent();
		}
	}
}

static const bool CanPutIntoRing (LPITEM ring, LPITEM item)
{
	return false;
}

bool CItem::CanPutInto (LPITEM item)
{
	if (item->GetType() == ITEM_BELT)
	{
		return this->GetSubType() == USE_PUT_INTO_BELT_SOCKET;
	}
	else if (item->GetType() == ITEM_RING)
	{
		return CanPutIntoRing (item, this);
	}
	else if (item->GetType() != ITEM_ARMOR)
	{
		return false;
	}

	uint32_t vnum = item->GetVnum();

	struct JewelAccessoryInfo
	{
		uint32_t jewel;
		uint32_t wrist;
		uint32_t neck;
		uint32_t ear;
	};

	const static JewelAccessoryInfo infos[] =
	{
		{ 50634, 14420, 16220, 17220 },
		{ 50635, 14500, 16500, 17500 },
		{ 50636, 14520, 16520, 17520 },
		{ 50637, 14540, 16540, 17540 },
		{ 50638, 14560, 16560, 17560 },
	};

	uint32_t item_type = (item->GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof (infos) / sizeof (infos[0]); i++)
	{
		const JewelAccessoryInfo& info = infos[i];

		switch (item->GetSubType())
		{

			case ARMOR_WRIST:
			{
				if (info.wrist == item_type)
				{
					if (info.jewel == GetVnum())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			break;

			case ARMOR_NECK:
			{
				if (info.neck == item_type)
				{
					if (info.jewel == GetVnum())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			break;

			case ARMOR_EAR:
			{
				if (info.ear == item_type)
				{
					if (info.jewel == GetVnum())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			break;
		}
	}
	
	if (item->GetSubType() == ARMOR_WRIST)
	{
		vnum -= 14000;
	}
	else if (item->GetSubType() == ARMOR_NECK)
	{
		vnum -= 16000;
	}
	else if (item->GetSubType() == ARMOR_EAR)
	{
		vnum -= 17000;
	}
	else
	{
		return false;
	}

	uint32_t type = vnum / 20;

	if (type < 0 || type > 11)
	{
		type = (vnum - 170) / 20;

		if (50623 + type != GetVnum())
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else if (item->GetVnum() >= 16210 && item->GetVnum() <= 16219)
	{
		if (50625 != GetVnum())
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else if (item->GetVnum() >= 16230 && item->GetVnum() <= 16239)
	{
		if (50626 != GetVnum())
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return 50623 + type == GetVnum();
}

bool CItem::CheckItemUseLevel(int32_t nLevel) const
{
	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		const auto& limit = m_pProto->aLimits[i];
		
		if (limit.bType == LIMIT_LEVEL)
		{
			return limit.lValue <= nLevel;
		}
	}
	
	return true;
}

int32_t CItem::FindApplyValue(uint8_t bApplyType) const
{
	if (!m_pProto) 
	{
		return 0;
	}

	for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		const auto& apply = m_pProto->aApplies[i];
		
		if (apply.bType == bApplyType)
		{
			return apply.lValue;
		}
	}

	return 0;
}

void CItem::CopySocketTo(LPITEM pItem)
{
	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		pItem->m_alSockets[i] = m_alSockets[i];
	}
}

int32_t CItem::GetAccessorySocketGrade()
{
   	return MINMAX(0, GetSocket(0), GetAccessorySocketMaxGrade()); 
}

int32_t CItem::GetAccessorySocketMaxGrade()
{
   	return MINMAX(0, GetSocket(1), ITEM_ACCESSORY_SOCKET_MAX_NUM);
}

int32_t CItem::GetAccessorySocketDownGradeTime()
{
	return MINMAX(0, GetSocket(2), aiAccessorySocketDegradeTime[GetAccessorySocketGrade()]);
}

void CItem::AttrLog() const
{
    const char* pszIP = GetOwner() && GetOwner()->GetDesc() ? GetOwner()->GetDesc()->GetHostName() : "";

    for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
    {
        if (m_alSockets[i])
        {
            LogManager::instance().ItemLog(i, m_alSockets[i], 0, GetID(), "INFO_SOCKET", "", pszIP, GetOriginalVnum());
        }
    }

    for (int32_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
    {
        if (m_aAttr[i].bType)
        {
            LogManager::instance().ItemLog(i, m_aAttr[i].bType, m_aAttr[i].sValue, GetID(), "INFO_ATTR", "", pszIP, GetOriginalVnum());
        }
    }
}

int32_t CItem::GetLevelLimit() const
{
    for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
    {
        if (m_pProto->aLimits[i].bType == LIMIT_LEVEL)
        {
            return m_pProto->aLimits[i].lValue;
        }
    }
	
    return 0;
}

bool CItem::OnAfterCreatedItem()
{
    if (GetProto()->cLimitRealTimeFirstUseIndex != -1 && GetSocket(1) != 0)
    {
        StartRealTimeExpireEvent();
    }

    return true;
}

bool CItem::IsDragonSoul() const
{
	return GetType() == ITEM_DS;
}

int32_t CItem::GiveMoreTime_Per(float fPercent)
{
   if (!IsDragonSoul()) 
   {
	   return 0;
   }

    uint32_t duration = DSManager::instance().GetDuration(this);
    int32_t remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
    int32_t given_time = static_cast<int32_t>(fPercent * duration / 100);

    if ((given_time + remain_sec) >= duration)
    {
        SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
        return duration - remain_sec;
    }

    SetSocket(ITEM_SOCKET_REMAIN_SEC, given_time + remain_sec);
    return given_time;
}

int32_t CItem::GiveMoreTime_Fix(uint32_t dwTime)
{
    if (!IsDragonSoul()) 
	{
		return 0;
	}

    uint32_t duration = DSManager::instance().GetDuration(this);
    int32_t remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);

    if ((dwTime + remain_sec) >= duration)
    {
        SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
        return duration - remain_sec;
    }

    SetSocket(ITEM_SOCKET_REMAIN_SEC, dwTime + remain_sec);
    return dwTime;
}

int32_t CItem::GetDuration()
{
	const auto* proto = GetProto();
	
	if (!proto)
	{
		return -1;
	}

	auto isRealTimeLimit = [proto](int32_t index) 
	{
		return LIMIT_REAL_TIME == proto->aLimits[index].bType;
	};

	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (isRealTimeLimit(i))
		{
			return proto->aLimits[i].lValue;
		}
	}

	if (proto->cLimitTimerBasedOnWearIndex != -1)
	{
		return proto->aLimits[proto->cLimitTimerBasedOnWearIndex].lValue;
	}

	return -1;
}

bool CItem::IsSameSpecialGroup(const LPITEM item) const
{
	return (this->GetVnum() == item->GetVnum()) || (GetSpecialGroup() && (item->GetSpecialGroup() == GetSpecialGroup()));
}
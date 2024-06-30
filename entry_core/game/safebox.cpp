#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "constants.h"
#include "safebox.h"
#include "packet.h"
#include "char.h"
#include "desc_client.h"
#include "item.h"
#include "item_manager.h"

namespace
{
	constexpr int ITEMS_PER_PAGE = 45;
	constexpr int PAGE_WIDTH = 5;
	constexpr int PAGE_HEIGHT = 9;

	constexpr uint8_t SafeboxPositionToPage(uint32_t position)
	{
		return static_cast<uint8_t>(position / ITEMS_PER_PAGE);
	}

	constexpr uint8_t SafeboxPositionToLocal(uint32_t position, uint8_t page)
	{
		return static_cast<uint8_t>(position - (ITEMS_PER_PAGE * page));
	}
}

CSafebox::CSafebox(LPCHARACTER pkChrOwner, int iSize, DWORD dwGold) : m_pkChrOwner(pkChrOwner), m_iSize(iSize), m_lGold(dwGold)
{
	assert(m_pkChrOwner != nullptr);
	memset(m_pkItems, 0, sizeof(m_pkItems));

	size_t numGrids = m_iSize / PAGE_HEIGHT;
	v_Grid.reserve(numGrids);

	std::generate_n(std::back_inserter(v_Grid), numGrids, []() 
	{
		return std::make_shared<CGrid>(PAGE_WIDTH, PAGE_HEIGHT);
	});

	m_bWindowMode = SAFEBOX;
}

CSafebox::~CSafebox()
{
	__Destroy();
}

void CSafebox::SetWindowMode(BYTE bMode)
{
	m_bWindowMode = bMode;
}

void CSafebox::__Destroy()
{
	for (int i = 0; i < SAFEBOX_MAX_NUM; ++i)
	{
		if (m_pkItems[i])
		{
			m_pkItems[i]->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(m_pkItems[i]);

			M2_DESTROY_ITEM(m_pkItems[i]->RemoveFromCharacter());
			m_pkItems[i] = NULL;
		}
	}
	
	v_Grid.clear();
}

bool CSafebox::Add(DWORD dwPos, LPITEM pkItem)
{
	if (!IsValidPosition(dwPos))
	{
		sys_err("SAFEBOX: item on wrong position at %d (size of grid = %d)", dwPos, GetGridTotalSize());
		return false;
	}

	pkItem->SetWindow(m_bWindowMode);
	pkItem->SetCell(m_pkChrOwner, dwPos);
	pkItem->Save();
	ITEM_MANAGER::instance().FlushDelayedSave(pkItem);

	const uint8_t pageIndex = SafeboxPositionToPage(dwPos);
	const uint8_t localPos = SafeboxPositionToLocal(dwPos, pageIndex);

	if (pageIndex >= v_Grid.size())
	{
		return false;
	}

	v_Grid[pageIndex]->Put(localPos, 1, pkItem->GetSize());

	m_pkItems[dwPos] = pkItem;

	TPacketGCItemSet pack;

	pack.header	= m_bWindowMode == SAFEBOX ? HEADER_GC_SAFEBOX_SET : HEADER_GC_MALL_SET;
	pack.Cell	= TItemPos(m_bWindowMode, dwPos);
	pack.vnum	= pkItem->GetVnum();
	pack.count	= pkItem->GetCount();
	pack.flags	= pkItem->GetFlag();
	pack.anti_flags	= pkItem->GetAntiFlag();
	thecore_memcpy(pack.alSockets, pkItem->GetSockets(), sizeof(pack.alSockets));
	thecore_memcpy(pack.aAttr, pkItem->GetAttributes(), sizeof(pack.aAttr));

	m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
	sys_log(1, "SAFEBOX: ADD %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return true;
}

LPITEM CSafebox::Get(DWORD dwPos)
{
    if (dwPos >= GetGridTotalSize())
	{
        return nullptr;
	}

	return m_pkItems[dwPos];
}

LPITEM CSafebox::Remove(DWORD dwPos)
{
	LPITEM pkItem = Get(dwPos);

	if (!pkItem)
		return NULL;

	const uint8_t pageIndex = SafeboxPositionToPage(dwPos);
	const uint8_t localPos = SafeboxPositionToLocal(dwPos, pageIndex);

	if (!v_Grid.empty() && pageIndex < v_Grid.size())
	{
		v_Grid[pageIndex]->Get(localPos, 1, pkItem->GetSize());
	}

	pkItem->RemoveFromCharacter();

	m_pkItems[dwPos] = nullptr;

	TPacketGCItemDel pack;

	pack.header	= m_bWindowMode == SAFEBOX ? HEADER_GC_SAFEBOX_DEL : HEADER_GC_MALL_DEL;
	pack.pos	= dwPos;

	m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
	sys_log(1, "SAFEBOX: REMOVE %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return pkItem;
}

void CSafebox::Save()
{
	TSafeboxTable t;

	memset(&t, 0, sizeof(TSafeboxTable));

	t.dwID = m_pkChrOwner->GetDesc()->GetAccountTable().id;
	t.dwGold = m_lGold;

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_SAVE, 0, &t, sizeof(TSafeboxTable));
	sys_log(1, "SAFEBOX: SAVE %s", m_pkChrOwner->GetName());
}

bool CSafebox::IsEmpty(DWORD dwPos, BYTE bSize)
{
	const uint8_t pageIndex = SafeboxPositionToPage(dwPos);
	const uint8_t localPos = SafeboxPositionToLocal(dwPos, pageIndex);

	if (pageIndex >= v_Grid.size())
	{
		return false;
	}

	return v_Grid[pageIndex]->IsEmpty(localPos, 1, bSize);
}

int32_t CSafebox::FindEmptySlot(uint8_t width, uint8_t height)
{
	for (size_t pageIndex = 0; pageIndex < v_Grid.size(); ++pageIndex)
	{
		int localPos = v_Grid[pageIndex]->FindBlank(width, height);

		if (localPos != -1)
		{
			return pageIndex * 45 + localPos;
		}
	}

	return -1;
}

int32_t CSafebox::GetEmptySafebox(uint8_t height)
{
	return FindEmptySlot(1, height);
}

void CSafebox::ChangeSize(int iSize)
{
	return;
}

LPITEM CSafebox::GetItem(BYTE bCell)
{
	if (bCell >= 5 * m_iSize)
	{
		sys_err("CHARACTER::GetItem: invalid item cell %d", bCell);
		return NULL;
	}

	return m_pkItems[bCell];
}

bool CSafebox::MoveItem(BYTE bCell, BYTE bDestCell, BYTE count)
{
	if (bCell == bDestCell) 
	{
		return false;
	}

	LPITEM item;

	int max_position = 5 * m_iSize;

	if (bCell >= max_position || bDestCell >= max_position)
		return false;

	if (!(item = GetItem(bCell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

	{
		LPITEM item2;

		if ((item2 = GetItem(bDestCell)) && item != item2 && item2->IsStackable() &&
				!IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
				item2->GetVnum() == item->GetVnum()) // 합칠 수 있는 아이템의 경우
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;

			if (count == 0)
				count = item->GetCount();

			count = MIN(200 - item2->GetCount(), count);

			if (item->GetCount() >= count)
				Remove(bCell);

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);

			sys_log(1, "SAFEBOX: STACK %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell, item2->GetName(), item2->GetCount());
			return true;
		}

		if (!IsEmpty(bDestCell, item->GetSize()))
			return false;

		const uint8_t pageIndex1 = SafeboxPositionToPage(bCell);
		const uint8_t localPos1 = SafeboxPositionToLocal(bCell, pageIndex1);
		const uint8_t pageIndex2 = SafeboxPositionToPage(bDestCell);
		const uint8_t localPos2 = SafeboxPositionToLocal(bDestCell, pageIndex2);

		if (pageIndex1 >= v_Grid.size() || pageIndex2 >= v_Grid.size())
		{
			return false;
		}

		v_Grid[pageIndex1]->Get(localPos1, 1, item->GetSize());

		if (!v_Grid[pageIndex2]->Put(localPos2, 1, item->GetSize()))
		{
			v_Grid[pageIndex1]->Put(localPos1, 1, item->GetSize());
			return false;
		}
		else
		{
			v_Grid[pageIndex2]->Get(localPos2, 1, item->GetSize());
			v_Grid[pageIndex1]->Put(localPos1, 1, item->GetSize());
		}

		sys_log(1, "SAFEBOX: MOVE %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell, item->GetName(), item->GetCount());

		Remove(bCell);
		Add(bDestCell, item);
	}

	return true;
}

uint32_t CSafebox::GetGridTotalSize() const
{
	return v_Grid.size() * ITEMS_PER_PAGE;
}

bool CSafebox::IsValidPosition(DWORD dwPos)
{
	if (v_Grid.empty())
	{
		return false;
	}

	if (dwPos >= GetGridTotalSize())
	{
		return false;
	}

	return true;
}
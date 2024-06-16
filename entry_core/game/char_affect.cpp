
#include "stdafx.h"

#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "packet.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "battle.h"
#include "guild.h"
#include "utils.h"
#include "locale_service.h"
#include "lua_incl.h"
#include "arena.h"
#include "horsename_manager.h"
#include "item.h"
#include "DragonSoul.h"
#include "skill.h"

#define IS_NO_SAVE_AFFECT(type) ((type) == AFFECT_WAR_FLAG || (type) == AFFECT_REVIVE_INVISIBLE || ((type) >= AFFECT_PREMIUM_START && (type) <= AFFECT_PREMIUM_END) || (type) == AFFECT_MOUNT_BONUS)
#define IS_NO_SAVE_AFFECT(type) ((type) == AFFECT_WAR_FLAG || (type) == AFFECT_REVIVE_INVISIBLE || ((type) >= AFFECT_PREMIUM_START && (type) <= AFFECT_PREMIUM_END))
#define IS_NO_CLEAR_ON_DEATH_AFFECT(type) ((type) == AFFECT_BLOCK_CHAT || ((type) >= 500 && (type) < 600))

void SendAffectRemovePacket(LPDESC d, DWORD pid, DWORD type, BYTE point)
{
	TPacketGCAffectRemove ptoc;
	ptoc.header	= HEADER_GC_AFFECT_REMOVE;
	ptoc.type		= type;
	ptoc.applyOn	= point;
	d->Packet(&ptoc, sizeof(TPacketGCAffectRemove));

	TPacketGDRemoveAffect ptod;
	ptod.playerId		= pid;
	ptod.type		= type;
	ptod.applyOn	= point;
	db_clientdesc->DBPacket(HEADER_GD_REMOVE_AFFECT, 0, &ptod, sizeof(ptod));
}

void SendAffectAddPacket(LPDESC d, CAffect * pkAff)
{
	TPacketGCAffectAdd ptoc;
	ptoc.header		= HEADER_GC_AFFECT_ADD;
	ptoc.element.type		= pkAff->type;
	ptoc.element.applyOn		= pkAff->applyOn;
	ptoc.element.applyValue	= pkAff->applyValue;
	ptoc.element.flag		= pkAff->flag;
	ptoc.element.duration		= pkAff->duration;
	ptoc.element.spCost		= pkAff->spCost;
	d->Packet(&ptoc, sizeof(TPacketGCAffectAdd));
}
////////////////////////////////////////////////////////////////////
// Affect

CAffect* CHARACTER::FindAffect(uint32_t type, uint8_t bApply) const
{
    for (const auto& pkAffect : m_list_pkAffect)
    {
        if (pkAffect->type == type && (bApply == APPLY_NONE || bApply == pkAffect->applyOn))
        {
            return pkAffect;
        }
    }
    return nullptr;
}


EVENTFUNC(affect_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "affect_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (!ch->UpdateAffect())
		return 0;
	else
		return passes_per_sec; // 1초
}

bool CHARACTER::UpdateAffect()
{
	// affect_event 에서 처리할 일은 아니지만, 1초짜리 이벤트에서 처리하는 것이
	// 이것 뿐이라 여기서 물약 처리를 한다.
	if (GetPoint(POINT_HP_RECOVERY) > 0)
	{
		if (GetMaxHP() <= GetHP())
		{
			PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
		}
		else
		{
			int iVal = 0;

			if (LC_IsYMIR())
			{
				iVal = MIN(GetPoint(POINT_HP_RECOVERY), GetMaxHP() * 9 / 100);
			}
			else
			{
				iVal = MIN(GetPoint(POINT_HP_RECOVERY), GetMaxHP() * 7 / 100);
			}

			PointChange(POINT_HP, iVal);
			PointChange(POINT_HP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_SP_RECOVERY) > 0)
	{
		if (GetMaxSP() <= GetSP())
			PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));
		else 
		{
			int iVal;

			if (!g_iUseLocale)
				iVal = MIN(GetPoint(POINT_SP_RECOVERY), GetMaxSP() * 7 / 100);
			else
				iVal = MIN(GetPoint(POINT_SP_RECOVERY), GetMaxSP() * 7 / 100);

			PointChange(POINT_SP, iVal);
			PointChange(POINT_SP_RECOVERY, -iVal);
		}
	}
	
	LPITEM weapon = GetWear(WEAR_WEAPON);

	if (!weapon || (weapon && (weapon->GetVnum() == 50201 || weapon->GetVnum() == 50202)))
	{
		if (IsAffectFlag(AFF_GEOMGYEONG))
		{
			RemoveAffect(SKILL_GEOMKYUNG);
		}

		if (IsAffectFlag(AFF_GWIGUM))
		{
			RemoveAffect(SKILL_GWIGEOM);
		}
	}

	if (GetPoint(POINT_HP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_HP, GetPoint(POINT_HP_RECOVER_CONTINUE));
	}

	if (GetPoint(POINT_SP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_SP, GetPoint(POINT_SP_RECOVER_CONTINUE));
	}

	AutoRecoveryItemProcess( AFFECT_AUTO_HP_RECOVERY );
	AutoRecoveryItemProcess( AFFECT_AUTO_SP_RECOVERY );

	// 스테미나 회복
	if (GetMaxStamina() > GetStamina())
	{
		int iSec = (get_dword_time() - GetStopTime()) / 3000;
		if (iSec)
			PointChange(POINT_STAMINA, GetMaxStamina()/1);    
	}


	// ProcessAffect는 affect가 없으면 true를 리턴한다.
	if (ProcessAffect())
		if (GetPoint(POINT_HP_RECOVERY) == 0 && GetPoint(POINT_SP_RECOVERY) == 0 && GetStamina() == GetMaxStamina())
		{
			m_pkAffectEvent = NULL;
			return false;
		}

	return true;
}

void CHARACTER::StartAffectEvent()
{
	if (m_pkAffectEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	m_pkAffectEvent = event_create(affect_event, info, passes_per_sec);
	sys_log(1, "StartAffectEvent %s %p %p", GetName(), this, get_pointer(m_pkAffectEvent));
}

void CHARACTER::ClearAffect(bool bSave)
{
	TAffectFlag afOld = m_afAffectFlag;
	WORD	wMovSpd = GetPoint(POINT_MOV_SPEED);
	WORD	wAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect * pkAff = *it;

		if (bSave)
		{
			if ( IS_NO_CLEAR_ON_DEATH_AFFECT(pkAff->type) || IS_NO_SAVE_AFFECT(pkAff->type) )
			{
				++it;
				continue;
			}

			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->type, pkAff->applyOn);
			}
		}

		ComputeAffect(pkAff, false);

		it = m_list_pkAffect.erase(it);
		CAffect::Release(pkAff);
	}

	if (afOld != m_afAffectFlag ||
			wMovSpd != GetPoint(POINT_MOV_SPEED) ||
			wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	CheckMaximumPoints();

	if (m_list_pkAffect.empty())
		event_cancel(&m_pkAffectEvent);
}

int CHARACTER::ProcessAffect()
{
	bool	bDiff	= false;
	CAffect	*pkAff	= NULL;

	//
	// 프리미엄 처리
	//
	for (int i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int aff_idx = i + AFFECT_PREMIUM_START;

		pkAff = FindAffect(aff_idx);

		if (!pkAff)
			continue;

		int remain = GetPremiumRemainSeconds(i);

		if (remain < 0)
		{
			RemoveAffect(aff_idx);
			bDiff = true;
		}
		else
			pkAff->duration = remain + 1;
	}

	////////// HAIR_AFFECT
	pkAff = FindAffect(AFFECT_HAIR);
	if (pkAff)
	{
		// IF HAIR_LIMIT_TIME() < CURRENT_TIME()
		if ( this->GetQuestFlag("hair.limit_time") < get_global_time())
		{
			// SET HAIR NORMAL
			this->SetPart(PART_HAIR, 0);
			// REMOVE HAIR AFFECT
			RemoveAffect(AFFECT_HAIR);
		}
		else
		{
			// INCREASE AFFECT DURATION
			++(pkAff->duration);
		}
	}
	////////// HAIR_AFFECT
	//

	CHorseNameManager::instance().Validate(this);

	TAffectFlag afOld = m_afAffectFlag;
	long lMovSpd = GetPoint(POINT_MOV_SPEED);
	long lAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it;

	it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		pkAff = *it;

		bool bEnd = false;

		if (pkAff->type >= GUILD_SKILL_START && pkAff->type <= GUILD_SKILL_END)
		{
			if (!GetGuild() || !GetGuild()->UnderAnyWar())
				bEnd = true;
		}

		if (pkAff->spCost > 0)
		{
			if (GetSP() < pkAff->spCost)
				bEnd = true;
			else
				PointChange(POINT_SP, -pkAff->spCost);
		}

		// AFFECT_DURATION_BUG_FIX
		// 무한 효과 아이템도 시간을 줄인다.
		// 시간을 매우 크게 잡기 때문에 상관 없을 것이라 생각됨.
		if ( --pkAff->duration <= 0 )
		{
			bEnd = true;
		}
		// END_AFFECT_DURATION_BUG_FIX

		if (bEnd)
		{
			it = m_list_pkAffect.erase(it);
			ComputeAffect(pkAff, false);
			bDiff = true;
			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->type, pkAff->applyOn);
			}

			CAffect::Release(pkAff);

			continue;
		}

		++it;
	}

	if (bDiff)
	{
		if (afOld != m_afAffectFlag ||
				lMovSpd != GetPoint(POINT_MOV_SPEED) ||
				lAttSpd != GetPoint(POINT_ATT_SPEED))
		{
			UpdatePacket();
		}

		CheckMaximumPoints();
	}

	if (m_list_pkAffect.empty())
		return true;

	return false;
}

void CHARACTER::SaveAffect()
{
	TPacketGDAddAffect p;

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect * pkAff = *it++;

		if (IS_NO_SAVE_AFFECT(pkAff->type))
			continue;

		sys_log(1, "AFFECT_SAVE: %u %u %d %d", pkAff->type, pkAff->applyOn, pkAff->applyValue, pkAff->duration);

		p.playerId			= GetPlayerID();
		p.affectElements.type		= pkAff->type;
		p.affectElements.applyOn		= pkAff->applyOn;
		p.affectElements.applyValue	= pkAff->applyValue;
		p.affectElements.flag		= pkAff->flag;
		p.affectElements.duration	= pkAff->duration;
		p.affectElements.spCost		= pkAff->spCost;
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}
}

EVENTINFO(load_affect_login_event_info)
{
	DWORD pid;
	DWORD count;
	char* data;

	load_affect_login_event_info()
	: pid( 0 )
	, count( 0 )
	, data( 0 )
	{
	}
};

EVENTFUNC(load_affect_login_event)
{
	load_affect_login_event_info* info = dynamic_cast<load_affect_login_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "load_affect_login_event_info> <Factor> Null pointer" );
		return 0;
	}

	DWORD dwPID = info->pid;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

	if (!ch)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	LPDESC d = ch->GetDesc();

	if (!d)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	if (d->IsPhase(PHASE_HANDSHAKE) ||
			d->IsPhase(PHASE_LOGIN) ||
			d->IsPhase(PHASE_SELECT) ||
			d->IsPhase(PHASE_DEAD) ||
			d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		sys_log(1, "Affect Load by Event");
		ch->LoadAffect(info->count, (TPacketAffectElement*)info->data);
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else
	{
		sys_err("input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
}

void CHARACTER::LoadAffect(DWORD dwCount, TPacketAffectElement * pElements)
{
	m_bIsLoadedAffect = false;

	if (!GetDesc()->IsPhase(PHASE_GAME))
	{
		if (test_server)
			sys_log(0, "LOAD_AFFECT: Creating Event", GetName(), dwCount);

		load_affect_login_event_info* info = AllocEventInfo<load_affect_login_event_info>();

		info->pid = GetPlayerID();
		info->count = dwCount;
		info->data = M2_NEW char[sizeof(TPacketAffectElement) * dwCount];
		thecore_memcpy(info->data, pElements, sizeof(TPacketAffectElement) * dwCount);

		event_create(load_affect_login_event, info, PASSES_PER_SEC(1));

		return;
	}

	ClearAffect(true);

	if (test_server)
		sys_log(0, "LOAD_AFFECT: %s count %d", GetName(), dwCount);

	TAffectFlag afOld = m_afAffectFlag;

	long lMovSpd = GetPoint(POINT_MOV_SPEED);
	long lAttSpd = GetPoint(POINT_ATT_SPEED);

	for (DWORD i = 0; i < dwCount; ++i, ++pElements)
	{

		if (pElements->type == SKILL_MUYEONG)
			continue;

		if (AFFECT_AUTO_HP_RECOVERY == pElements->type || AFFECT_AUTO_SP_RECOVERY == pElements->type)
		{
			LPITEM item = FindItemByID( pElements->flag );

			if (NULL == item)
				continue;

			item->Lock(true);
		}

		if (pElements->applyOn >= POINT_MAX_NUM)
		{
			sys_err("invalid affect data %s ApplyOn %u ApplyValue %d",
					GetName(), pElements->applyOn, pElements->applyValue);
			continue;
		}

		if (test_server)
		{
			sys_log(0, "Load Affect : Affect %s %d %d", GetName(), pElements->type, pElements->applyOn );
		}

		CAffect* pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);

		pkAff->type		= pElements->type;
		pkAff->applyOn		= pElements->applyOn;
		pkAff->applyValue	= pElements->applyValue;
		pkAff->flag		= pElements->flag;
		pkAff->duration	= pElements->duration;
		pkAff->spCost		= pElements->spCost;

		SendAffectAddPacket(GetDesc(), pkAff);

		ComputeAffect(pkAff, true);
	}

	if ( CArenaManager::instance().IsArenaMap(GetMapIndex()) == true )
	{
		RemoveGoodAffect();
	}
	
	RemoveAffect(AFFECT_MOUNT);
	RemoveAffect(AFFECT_MOUNT_BONUS);

	if (afOld != m_afAffectFlag || lMovSpd != GetPoint(POINT_MOV_SPEED) || lAttSpd != GetPoint(POINT_ATT_SPEED))
	{
		UpdatePacket();
	}

	StartAffectEvent();

	m_bIsLoadedAffect = true;

	ComputePoints();
	DragonSoul_Initialize();

	if (!IsDead())
	{
		PointChange(POINT_HP, GetMaxHP() - GetHP());
		PointChange(POINT_SP, GetMaxSP() - GetSP());
	}
}

bool CHARACTER::AddAffect(uint32_t type, uint8_t applyOn, int32_t applyValue, uint32_t flag, int32_t duration, int32_t spCost, bool bOverride, bool IsCube)
{
	if (type == AFFECT_BLOCK_CHAT && duration > 1)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;72;]");
	}

	if (duration == 0)
	{
		sys_err("Character::AddAffect duration == 0 type %d", duration, type);
		duration = 1;
	}

    if (!CheckSkillAffect(type, applyValue)) 
	{
        return false;
    }

	CAffect * pkAff = NULL;

	if (IsCube)
		pkAff = FindAffect(type,applyOn);
	else
		pkAff = FindAffect(type);

	if (flag == AFF_STUN)
	{
		if (m_posDest.x != GetX() || m_posDest.y != GetY())
		{
			m_posDest.x = m_posStart.x = GetX();
			m_posDest.y = m_posStart.y = GetY();
			battle_end(this);

			SyncPacket();
		}
	}

	// 이미 있는 효과를 덮어 쓰는 처리
	if (pkAff && bOverride)
	{
		ComputeAffect(pkAff, false); // ComputeAffect(Affect, !bOvveride, bOverride); 

		if (GetDesc())
			SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->type, pkAff->applyOn);
	}
	else
	{
		//
		// 새 에펙를 추가
		//
		// NOTE: 따라서 같은 type 으로도 여러 에펙트를 붙을 수 있다.
		// 
		pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);

	}

	sys_log(1, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), type, applyOn, applyValue, flag, duration);
	sys_log(0, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), type, applyOn, applyValue, flag, duration);

	pkAff->type	= type;
	pkAff->applyOn	= applyOn;
	pkAff->applyValue	= applyValue;
	pkAff->flag	= flag;
	pkAff->duration	= duration;
	pkAff->spCost	= spCost;

	WORD wMovSpd = GetPoint(POINT_MOV_SPEED);
	WORD wAttSpd = GetPoint(POINT_ATT_SPEED);

	ComputeAffect(pkAff, true);

	if (pkAff->flag || wMovSpd != GetPoint(POINT_MOV_SPEED) || wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	StartAffectEvent();

	if (IsPC())
	{
		SendAffectAddPacket(GetDesc(), pkAff);

		if (IS_NO_SAVE_AFFECT(pkAff->type))
			return true;

		TPacketGDAddAffect p;
		p.playerId			= GetPlayerID();
		p.affectElements.type		= pkAff->type;
		p.affectElements.applyOn		= pkAff->applyOn;
		p.affectElements.applyValue	= pkAff->applyValue;
		p.affectElements.flag		= pkAff->flag;
		p.affectElements.duration	= pkAff->duration;
		p.affectElements.spCost		= pkAff->spCost;
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}

	return true;
}

void CHARACTER::RefreshAffect()
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect * pkAff = *it++;
		ComputeAffect(pkAff, true);
	}
}

bool CHARACTER::CheckSkillAffect(uint32_t type, int32_t applyValue) 
{
	switch (type) 
	{
		case SKILL_HOSIN:
		case SKILL_REFLECT:
		case SKILL_GICHEON:
		case SKILL_JEONGEOP:
		case SKILL_KWAESOK:
		case SKILL_JEUNGRYEOK:
		{
			const CAffect * pkAffect = FindAffect(type);
			
			if (!pkAffect)
			{
				return true;
			}

			if (applyValue < pkAffect->applyValue) 
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SKILL_%s_LOW_YOUR_%ld%%_USE_%ld%%"), CSkillManager::instance().Get(type)->szName, applyValue, pkAffect->applyValue);
				return false;
			}
		}
		break;
		default:
		{
			return true;
		}
	}
	return true;
}

void CHARACTER::ComputeAffect(CAffect * pkAff, bool bAdd)
{
	if (bAdd && pkAff->type >= GUILD_SKILL_START && pkAff->type <= GUILD_SKILL_END)	
	{
		if (!GetGuild())
			return;

		if (!GetGuild()->UnderAnyWar())
			return;
	}
	if (pkAff->flag)
	{
		if (!bAdd)
			m_afAffectFlag.Reset(pkAff->flag);
		else
			m_afAffectFlag.Set(pkAff->flag);
	}

	if (bAdd)
		PointChange(pkAff->applyOn, pkAff->applyValue);
	else
		PointChange(pkAff->applyOn, -pkAff->applyValue);

	if (pkAff->type == SKILL_MUYEONG)
	{
		if (bAdd)
			StartMuyeongEvent();
		else
			StopMuyeongEvent();
	}
}

bool CHARACTER::RemoveAffect(CAffect* pkAff, bool updateNow) 
{
	if (!pkAff)
	{
		return false;
	}

	m_list_pkAffect.remove(pkAff);
	ComputeAffect(pkAff, false);

	if (updateNow) 
	{
		if (AFFECT_REVIVE_INVISIBLE != pkAff->type) 
		{
			ComputePoints();
		} 
		else 
		{
			UpdatePacket();
		}
	}
	else
	{
		UpdatePacket();
	}
	
	CheckMaximumPoints();

	if (test_server)
	{
		sys_log(0, "AFFECT_REMOVE: %s (flag %u apply: %u)", GetName(), pkAff->flag, pkAff->applyOn);
	}

	if (IsPC())
	{
		SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->type, pkAff->applyOn);
	}

	CAffect::Release(pkAff);
	return true;
}

bool CHARACTER::RemoveAffect(DWORD type)
{
	if (type == AFFECT_BLOCK_CHAT)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;389]");
	}
	
	bool flag = false;
	CAffect * pkAff;

while ((pkAff = FindAffect(type)))
{
	RemoveAffect(pkAff, false);
	flag = true;
}

	return flag;
}

bool CHARACTER::IsAffectFlag(DWORD dwAff) const
{
	return m_afAffectFlag.IsSet(dwAff);
}

void CHARACTER::RemoveGoodAffect() 
{
	const std::vector<uint16_t> goodAffects = 
	{
		AFFECT_MOV_SPEED, AFFECT_ATT_SPEED, AFFECT_STR, AFFECT_DEX,
		AFFECT_INT, AFFECT_CON, AFFECT_CHINA_FIREWORK, 
		SKILL_JEONGWI, SKILL_GEOMKYUNG, SKILL_CHUNKEON, 
		SKILL_EUNHYUNG, SKILL_GYEONGGONG, SKILL_GWIGEOM, 
		SKILL_TERROR, SKILL_JUMAGAP, SKILL_MANASHILED, 
		SKILL_HOSIN, SKILL_REFLECT, SKILL_KWAESOK, 
		SKILL_JEUNGRYEOK, SKILL_GICHEON
	};

	for (auto affect : goodAffects) 
	{
		const CAffect* pkAff = FindAffect(affect);
		
		if (pkAff) 
		{
			RemoveAffect(const_cast<CAffect*>(pkAff), false);
		}
	}
}

bool CHARACTER::RemoveFirstActiveAffect(uint32_t type)
{
	CAffect* pkAff = FindAffect(type);
	
	if (pkAff) 
	{
		RemoveAffect(pkAff);
		return true;
	}

	return false;
}

void CHARACTER::RemoveSkillAffects()
{
	static constexpr std::array<int16_t, 17> skills = 
	{
		SKILL_JEONGWI, 
		SKILL_GEOMKYUNG, 
		SKILL_CHUNKEON, 
		SKILL_EUNHYUNG, 
		SKILL_GYEONGGONG, 
		SKILL_GWIGEOM, 
		SKILL_TERROR, 
		SKILL_JUMAGAP, 
		SKILL_MANASHILED, 
		SKILL_HOSIN, 
		SKILL_REFLECT, 
		SKILL_KWAESOK, 
		SKILL_JEUNGRYEOK, 
		SKILL_GICHEON,
		SKILL_MUYEONG,		
	};

	for (const auto& skill : skills) 
	{
		RemoveFirstActiveAffect(skill);
	}
}

bool CHARACTER::IsGoodAffect(BYTE bAffectType) const
{
	switch (bAffectType)
	{
		case (AFFECT_MOV_SPEED):
		case (AFFECT_ATT_SPEED):
		case (AFFECT_STR):
		case (AFFECT_DEX):
		case (AFFECT_INT):
		case (AFFECT_CON):
		case (AFFECT_CHINA_FIREWORK):
		case (SKILL_JEONGWI):
		case (SKILL_GEOMKYUNG):
		case (SKILL_CHUNKEON):
		case (SKILL_EUNHYUNG):
		case (SKILL_GYEONGGONG):
		case (SKILL_GWIGEOM):
		case (SKILL_TERROR):
		case (SKILL_JUMAGAP):
		case (SKILL_MANASHILED):
		case (SKILL_HOSIN):
		case (SKILL_REFLECT):
		case (SKILL_KWAESOK):
		case (SKILL_JEUNGRYEOK):
		case (SKILL_GICHEON):
		{
			return true;
		}
	}
	
	return false;
}

void CHARACTER::RemoveBadAffect()
{
	const std::vector<uint16_t> badAffects = {AFFECT_POISON, AFFECT_FIRE, AFFECT_STUN, AFFECT_SLOW, SKILL_TUSOK};

	for (auto affect : badAffects) 
	{
		const CAffect* pkAff = FindAffect(affect);

		if (pkAff) 
		{
			RemoveAffect(const_cast<CAffect*>(pkAff), false);
		}
	}
}

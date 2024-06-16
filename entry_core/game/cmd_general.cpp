#include "stdafx.h"
#ifdef __FreeBSD__
#include <md5.h>
#else
#include "../../libthecore/include/xmd5.h"
#endif

#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "log.h"
#include "../../common/VnumHelper.h"

extern int g_server_id;
extern int g_nPortalLimitTime;

ACMD(do_user_horse_ride)
{
    static std::chrono::steady_clock::time_point lastCallTime;

    auto now = std::chrono::steady_clock::now();
    auto timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastCallTime);

    if (timeElapsed < COOLDOWN_HORSE_RIDE)
    {
        ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1;%d]", static_cast<int32_t>(COOLDOWN_HORSE_RIDE.count() - timeElapsed.count()));
        return;
    }

    lastCallTime = now;	
	
	if (ch->IsObserverMode() || ch->IsDead() || ch->IsStun())
	{
		return;
	}

	auto isRiding = ch->IsHorseRiding();
	auto hasMount = ch->GetMountVnum() != 0;
	auto hasHorse = ch->GetHorse() != nullptr;

	if (!isRiding)
	{
		if (hasMount)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;2]");
			return;
		}

		if (!hasHorse)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
			return;
		}
	}
	
	ch->ToggleRiding();
}

ACMD(do_user_horse_back)
{
	auto horse = ch->GetHorse();
	
    if (horse != nullptr)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;4]");
	}
    else if (ch->IsHorseRiding())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;5]");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
	}
}

ACMD(do_user_horse_feed)
{
	if (ch->GetMyShop())
	{
		return;
	}

    auto horse = ch->GetHorse();

    if (!horse)
	{
        auto message = ch->IsHorseRiding() ? "[LS;6]" : "[LS;3]";
        ch->ChatPacket(CHAT_TYPE_INFO, message);
        return;
	}
	
	if (ch->IsHorseDead())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;9]");
		return;
	}

	auto foodItemId = static_cast<uint32_t>(ch->GetHorseGrade() + 50054 - 1);
	auto itemTable = ITEM_MANAGER::instance().GetTable(foodItemId);
	auto itemName = itemTable->szLocaleName;

	if (ch->CountSpecifyItem(foodItemId) > 0)
	{
        ch->RemoveSpecifyItem(foodItemId, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;7;%s]", itemName);
	}
	else
	{
        ch->ChatPacket(CHAT_TYPE_INFO, "[LS;8;%s]", itemName);
	}
}

#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int		subcmd;
	int         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo() 
	: ch()
	, subcmd( 0 )
	, left_second( 0 )
	{
		::memset( szReason, 0, MAX_REASON_LEN );
	}
};

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int seconds;

	shutdown_event_data()
	: seconds( 0 )
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>( event->info );

	if ( info == NULL )
	{
		sys_err( "shutdown_event> <Factor> Null pointer" );
		return 0;
	}

	int * pSec = & (info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (--*pSec == -10)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--*pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf),"[LS;83;%d]", *pSec);
		SendNotice(buf);

		--*pSec;
		return passes_per_sec;
	}
}

void Shutdown(int iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), "[LS;82;%d]", iSec);

	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	if (ch == nullptr)
	{
		sys_err("Attempt to shutdown with null character pointer.");
		return;
	}

	if (!ch->IsGM())
	{
		sys_err("Unauthorized attempt to shutdown by %s.", ch->GetName());
		return;
	}

	TPacketGGShutdown p;
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );
	
	if ( info == NULL )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = NULL;

		if (true == LC_IsEurope() || true == LC_IsYMIR() || true == LC_IsKorea())
		{
			switch (info->subcmd)
			{
				case SCMD_LOGOUT:
				case SCMD_QUIT:
				case SCMD_PHASE_SELECT:
					{
						TPacketNeedLoginLogInfo acc_info;
						acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

						db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

						LogManager::instance().DetailLoginLog( false, ch );
					}
					break;
			}
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
			{
				if (d)
				{
					d->SetPhase(PHASE_CLOSE);
				}
				
				break;
			}

		case SCMD_QUIT:
		{
			if (d)
			{
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				d->DelayedDisconnect(2);
			}

			break;
		}

			case SCMD_PHASE_SELECT:
			{
				ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

				if (d)
				{
					d->SetPhase(PHASE_SELECT);
				}
				
				break;
			}
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;81;%d]", info->left_second);

		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{
	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;80]");
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;78]");
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;79]");
			break;
	}

	int nExitLimitTime = 10;

	if (ch->IsHack (false, true, nExitLimitTime) && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}
	
	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch		= ch;
				info->subcmd		= subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent	= event_create(timed_event, info, 1);
			}
			break;
	}
}

ACMD(do_mount)
{
}

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (NULL == ch->m_pkDeadEvent)
		return;

	int iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);

	if (subcmd != SCMD_RESTART_TOWN && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		if (!test_server)
		{
			if (iTimeToDead > 170)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;74;%d]", iTimeToDead - 170);
				return;
			}
		}
	}

	//PREVENT_HACK
	//DESC : 창고, 교환 창 후 포탈을 사용하는 버그에 이용될수 있어서
	//		쿨타임을 추가 
	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{
			if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;75;%d]", iTimeToDead - (180 - g_nPortalLimitTime));
				return;
			}
		}

		if (iTimeToDead > 173)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;76;%d]", iTimeToDead - 173);
			return;
		}
	}
	//END_PREVENT_HACK

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
	ch->StartRecoveryEvent();

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap * pMap = ch->GetWarMap();
		DWORD dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
					sys_log(0, "do_restart: restart town");
					PIXEL_POSITION pos;

					if (CWarMapManager::instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
					break;

				case SCMD_RESTART_HERE:
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
					break;
			}

			return;
		}
	}

	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
			sys_log(0, "do_restart: restart town");
			PIXEL_POSITION pos;

			if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(1);
			break;

		case SCMD_RESTART_HERE:
			sys_log(0, "do_restart: restart here");
			ch->RestartAtSamePos();
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(0);
			ch->ReviveInvisible(5);
			break;
	}
}

#define MAX_STAT 90

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;425]");
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;426]");
		return;
	}

	if (ch->GetPoint(POINT_STAT) <= 0)
		return;

	BYTE idx = 0;
	
	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	if (ch->GetRealPoint(idx) >= MAX_STAT)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
	ch->SetPoint(idx, ch->GetPoint(idx) + 1);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

	ch->PointChange(POINT_STAT, -1);
	ch->ComputePoints();
}

ACMD(do_pvp)
{	
	if (ch->GetArena() != NULL || CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;427]");
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

	if (pkVictim->GetArena() != NULL)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, "[LS;428]");
		return;
	}		
	
    if (pkVictim->GetLevel() < 15)
    {
        ch->ChatPacket(CHAT_TYPE_INFO, "[LS;307]");
        return;
    }		

	CPVPManager::instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;430]");
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;429]");
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch(vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:

			case SKILL_7_A_ANTI_TANHWAN:
			case SKILL_7_B_ANTI_AMSEOP:
			case SKILL_7_C_ANTI_SWAERYUNG:
			case SKILL_7_D_ANTI_YONGBI:

			case SKILL_8_A_ANTI_GIGONGCHAM:
			case SKILL_8_B_ANTI_YEONSA:
			case SKILL_8_C_ANTI_MAHWAN:
			case SKILL_8_D_ANTI_BYEURAK:

			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
				ch->SkillLevelUp(vnum);
				break;
		}
	}
}

ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

ACMD(do_safebox_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;84]");
		return;
	}

	if (!*arg2 || strlen(arg2)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;84]");
		return;
	}

	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;84]");
		return;
	}

	int iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO,"[LS;59]");
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10) // 10초에 한번만 요청 가능
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;60]");
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;43]");
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;85]");
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		CPartyManager::instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO,  "[LS;86]");
		pParty->Quit(ch->GetPlayerID());
	}
}

ACMD(do_close_shop)
{
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
}

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{
	auto playerGuild = ch->GetGuild();

	if (!playerGuild)
	{
		return;
	}

	if (playerGuild->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;87]");
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	uint8_t type = GUILD_WAR_TYPE_FIELD;

	if (!*arg1)
	{
		return;
	}

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type >= GUILD_WAR_TYPE_MAX_NUM)
		{
			type = GUILD_WAR_TYPE_FIELD;
		}
	}

	if (playerGuild->GetMasterPID() != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;88]");
		return;
	}

	auto opponentGuild  = CGuildManager::instance().FindGuildByName(arg1);

	if (!opponentGuild)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;89]");
		return;
	}

	switch (playerGuild->GetGuildWarState(opponentGuild ->GetID()))
	{
		case GUILD_WAR_NONE:
		{
			if (opponentGuild->UnderAnyWar())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;90]");
				return;
			}

			int32_t iWarPrice = KOR_aGuildWarInfo[type].iWarPrice;

			if (playerGuild->GetGuildMoney() < iWarPrice)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;91]");
				return;
			}

			if (opponentGuild->GetGuildMoney() < iWarPrice)
			{
				ch->ChatPacket(CHAT_TYPE_INFO,  "[LS;92]");
				return;
			}
		}
		break;

		case GUILD_WAR_SEND_DECLARE:
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;93]");
			return;
		}
		break;

		case GUILD_WAR_RECV_DECLARE:
		{
			if (opponentGuild->UnderAnyWar())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;87]");
				playerGuild->RequestRefuseWar(opponentGuild ->GetID());
				return;
			}
		}
		break;

		case GUILD_WAR_RESERVE:
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;93]");
			return;
		}
		break;

		case GUILD_WAR_END:
		{
			return;
		}

		default:
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;90]");
			playerGuild->RequestRefuseWar(opponentGuild ->GetID());
			return;
		}
	}

	if (!playerGuild->CanStartWar(type))
	{
		if (playerGuild->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;94]");
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (playerGuild->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;95;%d]", GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		
		return;
	}

	if (!opponentGuild ->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opponentGuild ->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;94]");
		}
		else if (opponentGuild ->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;96]");
		}
		
		return;
	}

	do
	{
		if (playerGuild->GetMasterCharacter() != nullptr)
		{
			break;
		}

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(playerGuild->GetMasterPID());

		if (pCCI != nullptr)
		{
			break;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;97]");
		playerGuild->RequestRefuseWar(opponentGuild ->GetID());
		return;

	} while (false);

	do
	{
		if (opponentGuild ->GetMasterCharacter() != nullptr)
		{
			break;
		}
		
		CCI *pCCI = P2P_MANAGER::instance().FindByPID(opponentGuild ->GetMasterPID());
		
		if (pCCI != nullptr)
		{
			break;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;98]");
		playerGuild->RequestRefuseWar(opponentGuild ->GetID());
		return;

	} while (false);

	playerGuild->RequestDeclareWar(opponentGuild ->GetID(), type);
}

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;431]");
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;89]");
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
	ch->DetailLog();
}

ACMD(do_monsterlog)
{
	ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	BYTE mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;101]");
		return;
	}

	char firstArgument[256], secondArgument[256];
	two_arguments(argument, firstArgument, sizeof(firstArgument), secondArgument, sizeof(secondArgument));

	if (!*firstArgument || !*secondArgument)
	{
		return;
	}

	const bool isFriendRequestDenied = std::tolower(firstArgument[0]) != 'y';
    const bool isFriendRequestProcessed = MessengerManager::instance().AuthToAdd(ch->GetName(), secondArgument, isFriendRequestDenied) != 0;

	if (isFriendRequestProcessed && isFriendRequestDenied)
	{
		if (auto targetCharacter = CHARACTER_MANAGER::instance().FindPC(secondArgument); targetCharacter)
		{
			targetCharacter->ChatPacket(CHAT_TYPE_INFO,"[LS;99;%s]", ch->GetName());
		}
	}
}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		BYTE flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
	auto mount = ch->GetWear(WEAR_COSTUME_MOUNT);

	auto unequipRideItem = [&](LPITEM item) 
	{
		if (item && item->IsRideItem()) 
		{
			ch->UnequipItem(item);
		}
	};

	unequipRideItem(ch->GetWear(WEAR_UNIQUE1));
	unequipRideItem(ch->GetWear(WEAR_UNIQUE2));
	unequipRideItem(mount);

	if (ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);
		
		if (ch->IsHorseRiding())
		{
			ch->StopRiding();
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;432]");
	}
}


ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(NULL);

		if (ch->GetArena() != NULL || ch->GetArenaObserverMode() == true)
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != NULL)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(NULL);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else
		{
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	if (ch->GetGMLevel() <= GM_PLAYER)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		DWORD vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;

		tch->SendEquipment(ch);
	}
}

ACMD(do_party_request)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;101]");
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;102]");
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	BYTE 	empire;
	int 	mapIndex;
	DWORD 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

		x = 0;
		y = 0;
	}

	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void __copy__(const GotoInfo& c_src)
	{
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

extern void BroadcastNotice(const char * c_pszBuf);

static std::string FN_point_string(int apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:
		{
			return "[LS;103;" + std::to_string(apply_number) + "]";
		}
		
		case POINT_MAX_SP:
		{
			return "[LS;104;%d]";
		}
		
		case POINT_HT:
		{
            return "[LS;105;" + std::to_string(apply_number) + "]";
		}
		
		case POINT_IQ:
		{
			return "[LS;106;%d]";
		}
		
		case POINT_ST:
		{
			return "[LS;107;%d]";
		}
		
		case POINT_DX:
		{	
			return "[LS;108;%d]";
		}
		
		case POINT_ATT_SPEED:
		{
			return "[LS;109;%d]";
		}
		
		case POINT_MOV_SPEED:
		{
			return "[LS;110;%d]";
		}
		
		case POINT_CASTING_SPEED:
		{
			return "[LS;111;%d]";
		}
		
		case POINT_HP_REGEN:
		{
			return "[LS;112;%d]";
		}
		
		case POINT_SP_REGEN:
		{
			return "[LS;113;%d]";
		}
		
		case POINT_POISON_PCT:
		{
			return "[LS;114;%d]";
		}
		
		case POINT_STUN_PCT:
		{
			return "[LS;115;%d]";
		}
		
		case POINT_SLOW_PCT:
		{
			return "[LS;116;%d]";
		}
		
		case POINT_CRITICAL_PCT:
		{
			return "[LS;117;%d%%]";
		}
		
		case POINT_RESIST_CRITICAL:
		{
			return "[LS;119;%d%%]";
		}
		
		case POINT_PENETRATE_PCT:
		{
			return "[LS;118;%d%%]";
		}
		
		case POINT_RESIST_PENETRATE:
		{
			return "[LS;120;%d%%]";
		}
		
		case POINT_ATTBONUS_HUMAN:
		{
			return "[LS;121;%d%%]";
		}
		
		case POINT_ATTBONUS_ANIMAL:	
		{
			return "[LS;122;%d%%]";
		}
		
		case POINT_ATTBONUS_ORC:	
		{
			return "[LS;123;%d%%]";
		}
		
		case POINT_ATTBONUS_MILGYO:
		{
			return "[LS;124;%d%%]";
		}
		
		case POINT_ATTBONUS_UNDEAD:
		{
			return "[LS;125;%d%%]";
		}
		
		case POINT_ATTBONUS_DEVIL:	
		{
			return "[LS;126;%d%%]";
		}
		
		case POINT_STEAL_HP:
		{
			return "[LS;127;%d%%]";
		}
		
		case POINT_STEAL_SP:	
		{		
			return "[LS;128;%d%%]";
		}
		
		case POINT_MANA_BURN_PCT:	
		{
			return "[LS;129;%d%%]";
		}
		
		case POINT_DAMAGE_SP_RECOVER:
		{
			return "[LS;130;%d%%]";
		}
		
		case POINT_BLOCK:			
		{
			return "[LS;131;%d%%]";
		}
		
		case POINT_DODGE:
		{
			return "[LS;132;%d%%]";
		}
		
		case POINT_RESIST_SWORD:
		{
			return "[LS;133;%d%%]";
		}
		
		case POINT_RESIST_TWOHAND:
		{
			return "[LS;134;%d%%]";
		}
		
		case POINT_RESIST_DAGGER:
		{
			return "[LS;135;%d%%]";
		}
		
		case POINT_RESIST_BELL:	
		{		
			return "[LS;136;%d%%]";
		}
		
		case POINT_RESIST_FAN:		
		{
			return "[LS;137;%d%%]";
		}
		
		case POINT_RESIST_BOW:	
		{		
			return "[LS;138;%d%%]";
		}
		
		case POINT_RESIST_FIRE:	
		{
			return "[LS;139;%d%%]";
		}
		
		case POINT_RESIST_ELEC:
		{
			return "[LS;140;%d%%]";
		}
		
		case POINT_RESIST_MAGIC:	
		{
			return "[LS;141;%d%%]";
		}
		
		case POINT_RESIST_WIND:	
		{
			return "[LS;142;%d%%]";
		}
		
		case POINT_RESIST_ICE:
		{
			return "[LS;143;%d%%]";
		}
		
		case POINT_RESIST_EARTH:
		{
			return "[LS;144;%d%%]";
		}
		
		case POINT_RESIST_DARK:
		{
			return "[LS;145;%d%%]";
		}
		
		case POINT_REFLECT_MELEE:
		{
			return "[LS;146;%d%%]";
		}
		
		case POINT_REFLECT_CURSE:
		{
			return "[LS;147;%d%%]";
		}
		
		case POINT_POISON_REDUCE:
		{
			return "[LS;148;%d%%]";
		}
		
		case POINT_KILL_SP_RECOVER:
		{
			return "[LS;149;%d%%]";
		}
		
		case POINT_EXP_DOUBLE_BONUS:
		{
			return "[LS;150;%d%%]";
		}
		
		case POINT_GOLD_DOUBLE_BONUS:
		{
			return "[LS;151;%d%%]";
		}
		
		case POINT_ITEM_DROP_BONUS:
		{
			return "[LS;152;%d%%]";
		}
		
		case POINT_POTION_BONUS:
		{
			return "[LS;153;%d%%]";
		}
		
		case POINT_KILL_HP_RECOVERY:
		{
			return "[LS;154;%d%%]";
		}
		
		case POINT_ATT_GRADE_BONUS:
		{
			return "[LS;155;%d%]";
		}
		
		case POINT_DEF_GRADE_BONUS:
		{
			return "[LS;156;%d%]";
		}
		
		case POINT_MAGIC_ATT_GRADE:
		{
			return "[LS;157;%d%]";
		}
		
		case POINT_MAGIC_DEF_GRADE:
		{
			return "[LS;158;%d%]";
		}
		
		case POINT_MAX_STAMINA:
		{
			return "[LS;159;%d%]";
		}
		
		case POINT_ATTBONUS_WARRIOR:
		{
			return "[LS;160;%d%%]";
		}
		
		case POINT_ATTBONUS_ASSASSIN:
		{
			return "[LS;161;%d%%]";
		}
		
		case POINT_ATTBONUS_SURA:
		{
			return "[LS;162;%d%%]";
		}
		
		case POINT_ATTBONUS_SHAMAN:
		{
			return "[LS;163;%d%%]";
		}
		
		case POINT_ATTBONUS_MONSTER:
		{
			return "[LS;164;%d%%]";
		}
		
		case POINT_MALL_ATTBONUS:
		{
			return "[LS;165;%d%%]";
		}
		
		case POINT_MALL_DEFBONUS:
		{
			return "[LS;166;%d%%]";
		}
		
		case POINT_MALL_EXPBONUS:
		{
			return "[LS;167;%d%%]";
		}

		case POINT_MALL_ITEMBONUS:
		{
			return "[LS;168;%d]";
		}
		
		case POINT_MALL_GOLDBONUS:
		{
			return "[LS;169;%d]";
		}
		
		case POINT_MAX_HP_PCT:
		{
			return "[LS;170;%d%%]";
		}
		
		case POINT_MAX_SP_PCT:
		{
			return "[LS;171;%d%%]";
		}
		
		case POINT_SKILL_DAMAGE_BONUS:
		{
			return "[LS;172;%d%%]";
		}
		
		case POINT_NORMAL_HIT_DAMAGE_BONUS:
		{
			return "[LS;173;%d%%]";
		}
		
		case POINT_SKILL_DEFEND_BONUS:
		{
			return "[LS;174;%d%%]";
		}
		
		case POINT_NORMAL_HIT_DEFEND_BONUS:
		{
			return "[LS;175;%d%%]";
		}
		
		case POINT_RESIST_WARRIOR:
		{
			return "[LS;176;%d%%]";
		}
		
		case POINT_RESIST_ASSASSIN:
		{
			return "[LS;177;%d%%]";
		}
		
		case POINT_RESIST_SURA:
		{
			return "[LS;178;%d%%]";
		}
		
		case POINT_RESIST_SHAMAN:
		{
			return "[LS;179;%d%%]";
		}
		
		default:
		{
			return "error %d";
		}
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char *buf, size_t bufsiz)
{
	if (NULL == ch || NULL == buf)
		return false;

	CAffect* aff = NULL;
	time_t expire = 0;
	struct tm ltm;
	int	year, mon, day;
	int	offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (NULL == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_global_time())
		return false;

	const std::string pointString = FN_point_string(aff->applyOn);
	
	if (pointString.empty())
	{
		return false;
	}

	offset = snprintf(buf, bufsiz, pointString.c_str(), aff->applyValue);
	
	if (offset < 0 || offset >= (int) bufsiz)
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year	= ltm.tm_year + 1900;
	mon		= ltm.tm_mon + 1;
	day		= ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, "[LS;180;%d;%d;%d]", year, mon, day);

	return true;
}

ACMD(do_costume)
{
	char buf[512];
	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
	CItem* pMount = ch->GetWear(WEAR_COSTUME_MOUNT);

	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;435]");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;436]", itemName);

		for (int i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.bType)
			{
				const std::string& pointString = FN_point_string(attr.bType);
				
				if (pointString.empty())
				{
					continue;
				}
			
				snprintf(buf, bufferSize, pointString.c_str(), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;436]", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;438]", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}
	
	if (pMount)
	{
		const char* itemName = pMount->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;439]", itemName);

		if (pMount->IsEquipped() && arg1[0] == 'm')
			ch->UnequipItem(pMount);
	}
}

ACMD(do_hair)
{
	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	int	index = 0;
	int	count		= 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int i = 0; i < count; ++i)
	{
		if (index >= INVENTORY_MAX_NUM)
			break;

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "ekwipunek [%d] = %s", index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

//gift notify quest command
ACMD(do_gift)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

ACMD(do_cube)
{
	if (!ch->CanDoCube())
		return;

	int cube_index = 0, inven_index = 0;
	const char *line;

	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)
	// /cube r_info     ==> (Client -> Server) 현재 NPC가 만들 수 있는 레시피 요청
	//					    (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//
	// /cube r_info 3   ==> (Client -> Server) 현재 NPC가 만들수 있는 레시피 중 3번째 아이템을 만드는 데 필요한 정보를 요청
	// /cube r_info 3 5 ==> (Client -> Server) 현재 NPC가 만들수 있는 레시피 중 3번째 아이템부터 이후 5개의 아이템을 만드는 데 필요한 재료 정보를 요청
	//					   (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
		case 'o':	// open
			Cube_open(ch);
			break;

		case 'c':	// close
			Cube_close(ch);
			break;

		case 'l':	// list
			Cube_show_list(ch);
			break;

		case 'a':	// add cue_index inven_index
			{
				if (0 == arg2[0] || !isdigit(*arg2) ||
					0 == arg3[0] || !isdigit(*arg3))
					return;

				str_to_number(cube_index, arg2);
				str_to_number(inven_index, arg3);
				Cube_add_item (ch, cube_index, inven_index);
			}
			break;

		case 'd':	// delete
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;

				str_to_number(cube_index, arg2);
				Cube_delete_item (ch, cube_index);
			}
			break;

		case 'm':
			Cube_make (ch);
			break;

		default:
			return;
	}
}

ACMD(do_in_game_mall)
{
}

// 주사위
ACMD(do_dice) 
{
	char arg1[256], arg2[256];
	int start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int n = number(start, end);
	
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, "[LS;433;%s;%d;%d;%d]", ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;434;%d;%d;%d]", n, start, end);
}

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

ACMD(do_ride)
{
	const auto jazdaTime = static_cast<int>((get_dword_time() - ch->GetLastMountTime()) / 1000 + 0.5);

	if (jazdaTime < 1 || ch->IsDead() || ch->IsStun() || ch->GetMapIndex() == 113 || ch->IsPolymorphed())
	{
		return;
	}

	if(ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;11]");
		return;
	}
	
	if (ch->IsHorseRiding())
	{
		ch->StopRiding();
		return;
	}

	if (ch->GetHorse())
	{
		ch->StartRiding();
		return;
	}

	for (BYTE i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		auto item = ch->GetInventoryItem(i);
		
		if (item && item->IsRideItem() && !ch->GetWear(WEAR_UNIQUE1) && !ch->GetWear(WEAR_UNIQUE2) && !ch->GetWear(WEAR_COSTUME_MOUNT))
		{
			ch->UseItem(TItemPos(INVENTORY, i));
			return;
		}
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
}
#include "stdafx.h"
#include "utils.h"
#include "char.h"
#include "OXEvent.h"
#include "questmanager.h"
#include "questlua.h"
#include "config.h"
#include "locale_service.h"
#include "cmd.h"

ACMD(do_oxevent_show_quiz)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;561]");
	COXEventManager::instance().ShowQuizList(ch);
}

ACMD(do_oxevent_log)
{
	if ( COXEventManager::instance().LogWinner() == false )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;558]");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;559]");
	}
}

ACMD(do_oxevent_get_attender)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;560;%d]", COXEventManager::instance().GetAttenderCount());
}


#include "stdafx.h"
#include "constants.h"
#include "gm.h"
#include "messenger_manager.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "log.h"
#include "config.h"
#include "p2p.h"
#include "crc32.h"
#include "char.h"
#include "char_manager.h"
#include "questmanager.h"

static char	__account[CHARACTER_NAME_MAX_LEN * 2 + 1];
static char	__companion[CHARACTER_NAME_MAX_LEN * 2 + 1];

void MessengerManager::Initialize() {}
void MessengerManager::Destroy() {}

void MessengerManager::P2PLogin(MessengerManager::accountKeyReference account)
{
	Login(account);
}

void MessengerManager::P2PLogout(MessengerManager::accountKeyReference account)
{
	Logout(account);
}

void MessengerManager::Login(MessengerManager::accountKeyReference account)
{
	if (m_set_loginAccount.find(account) != m_set_loginAccount.end())
	{
		return;
	}
	
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	
	if (account.compare(__account))
	{
		return;
	}

	DBManager::Instance().FuncQuery(std::bind(&MessengerManager::LoadList, this, std::placeholders::_1), "SELECT account, companion FROM messenger_list WHERE account='%s'", __account);

	m_set_loginAccount.insert(account);
}

void MessengerManager::LoadList(SQLMsg * msg)
{
	if (NULL == msg)
		return;

	if (NULL == msg->Get())
		return;

	if (msg->Get()->uiNumRows == 0)
		return;

	std::string account;

	for (uint i = 0; i < msg->Get()->uiNumRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		if (row[0] && row[1])
		{
			if (account.length() == 0)
				account = row[0];

			m_Relation[row[0]].insert(row[1]);
			m_InverseRelation[row[1]].insert(row[0]);
		}
	}

	SendList(account);

	std::set<MessengerManager::AccountKey>::iterator it;

	for (it = m_InverseRelation[account].begin(); it != m_InverseRelation[account].end(); ++it)
		SendLogin(*it, account);
}


void MessengerManager::Logout(MessengerManager::accountKeyReference account)
{
	if (m_set_loginAccount.find(account) == m_set_loginAccount.end())
		return;

	m_set_loginAccount.erase(account);

	std::set<MessengerManager::AccountKey>::iterator it;

	for (it = m_InverseRelation[account].begin(); it != m_InverseRelation[account].end(); ++it)
	{
		SendLogout(*it, account);
	}

	std::map<AccountKey, std::set<AccountKey> >::iterator it2 = m_Relation.begin();

	while (it2 != m_Relation.end())
	{
		it2->second.erase(account);
		++it2;
	}

	m_Relation.erase(account);
}

void MessengerManager::RequestToAdd(LPCHARACTER ch, LPCHARACTER target)
{
	if (!ch->IsPC() || !target->IsPC())
		return;
	
	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;712]");
		return;
	}

	if (quest::CQuestManager::instance().GetPCForce(target->GetPlayerID())->IsRunning() == true)
		return;

	if (IsAlreadyFriends(ch->GetName(), target->GetName()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;713;%s;%s]", ch->GetName(), target->GetName());
		return;
	}

	DWORD dw1 = GetCRC32(ch->GetName(), strlen(ch->GetName()));
	DWORD dw2 = GetCRC32(target->GetName(), strlen(target->GetName()));

	char buf[64];
	snprintf(buf, sizeof(buf), "%u:%u", dw1, dw2);
	DWORD dwComplex = GetCRC32(buf, strlen(buf));

	m_set_requestToAdd.insert(dwComplex);

	target->ChatPacket(CHAT_TYPE_COMMAND, "messenger_auth %s", ch->GetName());
}

bool MessengerManager::AuthToAdd(MessengerManager::accountKeyReference account, MessengerManager::accountKeyReference companion, bool bDeny)
{
	DWORD dw1 = GetCRC32(companion.c_str(), companion.length());
	DWORD dw2 = GetCRC32(account.c_str(), account.length());

	char buf[64];
	snprintf(buf, sizeof(buf), "%u:%u", dw1, dw2);
	DWORD dwComplex = GetCRC32(buf, strlen(buf));

	if (m_set_requestToAdd.find(dwComplex) == m_set_requestToAdd.end())
	{
		sys_log(0, "MessengerManager::AuthToAdd : request not exist %s -> %s", companion.c_str(), account.c_str());
		return false;
	}

	m_set_requestToAdd.erase(dwComplex);

	if (!bDeny)
	{
		AddToList(companion, account);
		AddToList(account, companion);
	}
	
	return true;
}

void MessengerManager::__AddToList(MessengerManager::accountKeyReference account, MessengerManager::accountKeyReference companion)
{
	m_Relation[account].insert(companion);
	m_InverseRelation[companion].insert(account);

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : NULL;

	if (d)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;266;%s]", companion.c_str());
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(companion.c_str());

	if (tch)
	{
		SendLogin(account, companion);
	}
	else
	{
		SendLogout(account, companion);
	}
}

void MessengerManager::AddToList(MessengerManager::accountKeyReference account, MessengerManager::accountKeyReference companion)
{
	if (companion.size() == 0)
	{
		return;
	}

	if (m_Relation[account].find(companion) != m_Relation[account].end())
	{
		return;
	}

	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	DBManager::Instance().EscapeString(__companion, sizeof(__companion), companion.c_str(), companion.size());
	
	if (account.compare(__account) || companion.compare(__companion))
	{
		return;
	}

	DBManager::Instance().Query("INSERT INTO messenger_list VALUES ('%s', '%s')",  __account, __companion);

	__AddToList(account, companion);

	TPacketGGMessenger p2ppck;

	p2ppck.bHeader = HEADER_GG_MESSENGER_ADD;
	strlcpy(p2ppck.szAccount, account.c_str(), sizeof(p2ppck.szAccount));
	strlcpy(p2ppck.szCompanion, companion.c_str(), sizeof(p2ppck.szCompanion));
	P2P_MANAGER::instance().Send(&p2ppck, sizeof(TPacketGGMessenger));
}

void MessengerManager::ResetChatPacketState(const std::string& account)
{
    m_mapSentChatPackets[account] = false;
}

void MessengerManager::__RemoveFromList(MessengerManager::accountKeyReference account, MessengerManager::accountKeyReference companion)
{
    sys_log(0, "__RemoveFromList: Removing companion %s from account %s", companion.c_str(), account.c_str());

    m_Relation[account].erase(companion);
    m_InverseRelation[companion].erase(account);
    
    m_Relation[companion].erase(account);
    m_InverseRelation[account].erase(companion);

    LPCHARACTER initiatingPlayer = CHARACTER_MANAGER::instance().FindPC(account.c_str());
    LPDESC initiatingPlayerDesc = initiatingPlayer ? initiatingPlayer->GetDesc() : nullptr;

    if (initiatingPlayerDesc) 
	{
    
        sys_log(0, "__RemoveFromList: Sending packet to initiating player %s", account.c_str());
        initiatingPlayer->ChatPacket(CHAT_TYPE_INFO, "[LS;265;%s]", companion.c_str());

        // Send packet to remove friend from initiating player
        const BYTE HeaderValue = HEADER_GC_MESSENGER;
        const BYTE SubheaderValue = MESSENGER_SUBHEADER_GC_REMOVE_FRIEND;
        const WORD PacketSize = static_cast<WORD>(sizeof(TPacketGCMessenger) + sizeof(BYTE) + companion.size());

        TPacketGCMessenger removeFriendPacket {HeaderValue, PacketSize, SubheaderValue};
        BYTE companionSizeByte = static_cast<BYTE>(companion.size());

        initiatingPlayerDesc->BufferedPacket(&removeFriendPacket, sizeof(removeFriendPacket));
        initiatingPlayerDesc->BufferedPacket(&companionSizeByte, sizeof(BYTE));
        initiatingPlayerDesc->Packet(companion.data(), companion.size());
		

		TPacketGGMessenger p2ppck;

		p2ppck.bHeader = HEADER_GG_MESSENGER_REMOVE;
		strlcpy(p2ppck.szAccount, account.c_str(), sizeof(p2ppck.szAccount));
		strlcpy(p2ppck.szCompanion, companion.c_str(), sizeof(p2ppck.szCompanion));
		P2P_MANAGER::instance().Send(&p2ppck, sizeof(TPacketGGMessenger));		
    }

    LPCHARACTER targetFriend = CHARACTER_MANAGER::instance().FindPC(companion.c_str());
    
    if (targetFriend && targetFriend->GetDesc())
    {
        sys_log(0, "__RemoveFromList: Sending packet to target friend %s", companion.c_str());

        // Send packet to remove friend from target friend
        const BYTE HeaderValue = HEADER_GC_MESSENGER;
        const BYTE SubheaderValue = MESSENGER_SUBHEADER_GC_REMOVE_FRIEND;
        const WORD PacketSize = static_cast<WORD>(sizeof(TPacketGCMessenger) + sizeof(BYTE) + account.size());

        TPacketGCMessenger removeFriendPacket {HeaderValue, PacketSize, SubheaderValue};
        BYTE accountSizeByte = static_cast<BYTE>(account.size());

        targetFriend->GetDesc()->BufferedPacket(&removeFriendPacket, sizeof(removeFriendPacket));
        targetFriend->GetDesc()->BufferedPacket(&accountSizeByte, sizeof(BYTE));
        targetFriend->GetDesc()->Packet(account.data(), account.size());

        targetFriend->ChatPacket(CHAT_TYPE_INFO, "[LS;267;%s]", account.c_str());
    }
}


void MessengerManager::RemoveFromList(MessengerManager::accountKeyReference account, MessengerManager::accountKeyReference companion)
{
	if (companion.empty())
	{
		return;
	}
	
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	DBManager::Instance().EscapeString(__companion, sizeof(__companion), companion.c_str(), companion.size());
	
	if (account != __account || companion != __companion)
	{
		return;
	}

	DBManager::Instance().Query("DELETE FROM messenger_list WHERE (account='%s' AND companion='%s') OR (account='%s' AND companion='%s')", __account, __companion, __companion, __account);

	__RemoveFromList(account, companion);
}


void MessengerManager::RemoveAllList(accountKeyReference account)
{
	std::set<AccountKey> company(m_Relation[account]);
	std::set<AccountKey> inverseCompany(m_InverseRelation[account]);

	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	
	if (account.compare(__account) != 0)
	{
		return;
	}

	DBManager::Instance().Query("DELETE FROM messenger_list WHERE account='%s' OR companion='%s'", __account, __account);

	for (const auto& companion : company)
	{
		this->RemoveFromList(account, companion);
	}

	for (const auto& companion : inverseCompany)
	{
		this->RemoveFromList(companion, account);
	}

	company.clear();
	inverseCompany.clear();

	m_Relation.erase(account);
	m_InverseRelation.erase(account);
}

void MessengerManager::SendList(MessengerManager::accountKeyReference account)

{
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(account.c_str());

	if (!ch)
		return;

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	if (m_Relation.find(account) == m_Relation.end())
		return;

	if (m_Relation[account].empty())
		return;

	TPacketGCMessenger pack;

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_LIST;
	pack.size = sizeof(TPacketGCMessenger);

	TPacketGCMessengerListOffline pack_offline;
	TPacketGCMessengerListOnline pack_online;

	TEMP_BUFFER buf(128 * 1024); // 128k

	itertype(m_Relation[account]) it = m_Relation[account].begin(), eit = m_Relation[account].end();

	while (it != eit)
	{
		if (m_set_loginAccount.find(*it) != m_set_loginAccount.end())
		{
			pack_online.connected = 1;

			// Online
			pack_online.length = it->size();

			buf.write(&pack_online, sizeof(TPacketGCMessengerListOnline));
			buf.write(it->c_str(), it->size());
		}
		else
		{
			pack_offline.connected = 0;

			// Offline
			pack_offline.length = it->size();

			buf.write(&pack_offline, sizeof(TPacketGCMessengerListOffline));
			buf.write(it->c_str(), it->size());
		}

		++it;
	}

	pack.size += buf.size();

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->Packet(buf.read_peek(), buf.size());
}

void MessengerManager::SendLogin(MessengerManager::accountKeyReference account, MessengerManager::accountKeyReference companion)
{
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : NULL;

	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	if (ch->GetGMLevel() == GM_PLAYER && gm_get_level(companion.c_str()) != GM_PLAYER)
		return;

	BYTE bLen = companion.size();

	TPacketGCMessenger pack;

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_LOGIN;
	pack.size = sizeof(TPacketGCMessenger) + sizeof(BYTE) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(BYTE));
	d->Packet(companion.c_str(), companion.size());
}

void MessengerManager::SendLogout(MessengerManager::accountKeyReference account, MessengerManager::accountKeyReference companion)
{
	if (!companion.size())
		return;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : NULL;

	if (!d)
		return;

	BYTE bLen = companion.size();

	TPacketGCMessenger pack;

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_LOGOUT;
	pack.size = sizeof(TPacketGCMessenger) + sizeof(BYTE) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(BYTE));
	d->Packet(companion.c_str(), companion.size());
}

bool MessengerManager::IsAlreadyFriends(accountKeyReference account, accountKeyReference companion)
{
	auto itAccount = m_Relation.find(account);
	if (itAccount != m_Relation.end() && itAccount->second.find(companion) != itAccount->second.end())
	{
		return true;
	}

	auto itCompanion = m_Relation.find(companion);
	if (itCompanion != m_Relation.end() && itCompanion->second.find(account) != itCompanion->second.end())
	{
		return true;
	}

	return false;
}

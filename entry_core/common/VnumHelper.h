#pragma once

class CItemVnumHelper
{
	public:
		static const bool IsPhoenix(DWORD vnum)
		{
			return 53001 == vnum;
		}

		static const bool IsRamadanMoonRing(DWORD vnum)
		{
			return 71135 == vnum;
		}

		static const bool IsHalloweenCandy(DWORD vnum)
		{
			return 71136 == vnum;
		}

		static const bool IsHappinessRing(DWORD vnum)
		{
			return 71143 == vnum;
		}

		static const bool IsLovePendant(DWORD vnum)
		{
			return 71145 == vnum;
		}

		static const bool IsSpecialItem(DWORD vnum)
		{
			return IsRamadanMoonRing(vnum) || IsHalloweenCandy(vnum) || IsHappinessRing(vnum) || IsLovePendant(vnum);
		}
};

class CMobVnumHelper
{
	public:
		static bool IsMount(DWORD vnum)
		{
			static const std::set<DWORD> mountVnums = {20110, 20111, 20112, 20113, 20114, 20115, 20116, 20117, 20118, 20119, 20120};
			return mountVnums.contains(vnum);
		}

		static bool IsPhoenix(DWORD vnum)
		{
			return 34001 == vnum;
		}

		static bool IsIcePhoenix(DWORD vnum)
		{
			return 34003 == vnum;
		}

		static bool IsPetUsingPetSystem(DWORD vnum)
		{
			return (IsPhoenix(vnum) || IsReindeerYoung(vnum)) || IsIcePhoenix(vnum);
		}

		static bool IsReindeerYoung(DWORD vnum)
		{
			return 34002 == vnum;
		}

		static bool IsRamadanBlackHorse(DWORD vnum)
		{
			return 20119 == vnum || 20219 == vnum || 22022 == vnum;
		}
};
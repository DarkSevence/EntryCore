#include "stdafx.h"

#include "char.h"
#include "skill.h"

int32_t CHARACTER::CalculateMagicDamage(int32_t initialDamage, int32_t magicAttBonusPer, int32_t meleeMagicAttBonusPer) 
{
    const float magicAttBonusPercent = static_cast<float>(magicAttBonusPer);
    const float meleeMagicAttBonusPercent = static_cast<float>(meleeMagicAttBonusPer);
    
    const float totalBonusPercent = 100.0f + magicAttBonusPercent + meleeMagicAttBonusPercent;
    
    return static_cast<int32_t>(static_cast<float>(initialDamage) * totalBonusPercent / 100.0f + 0.5f);
}

//todo calc critical

// Używane od wersji 1.3 
int32_t CHARACTER::CalculatePenetrationPercent(int32_t initialPenetratePct, bool isPC, int32_t marriageBonus, float skillResistPenetratePower, int32_t resistPenetrate)
{
    int32_t iPenetratePct = initialPenetratePct;

    if (!isPC)
    {
        iPenetratePct += marriageBonus;
    }

    CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_RESIST_PENETRATE);

    if (pkSk != nullptr)
    {
        pkSk->SetPointVar("k", 1.0f * skillResistPenetratePower / 100.0f);
        iPenetratePct -= static_cast<int>(pkSk->kPointPoly.Eval());
    }

    iPenetratePct -= resistPenetrate;

    return iPenetratePct;
}

std::pair<int32_t, bool> CHARACTER::CalculatePenetrationDamage(int32_t iPenetratePct, int32_t dam, bool test_server, int32_t pointDefGrade, int32_t pointDefBonus) 
{
    bool IsPenetrate = false;
    
    if (iPenetratePct) 
    {
        if (number(1, 100) <= iPenetratePct) 
	   {
            IsPenetrate = true;
            dam += pointDefGrade * (100 + pointDefBonus) / 100;
        }
    }

    return {dam, IsPenetrate};
}
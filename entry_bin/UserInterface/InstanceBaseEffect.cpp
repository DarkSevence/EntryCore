#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonTextTail.h"
#include "AbstractApplication.h"
#include "AbstractPlayer.h"
#include "PythonPlayer.h"
#include "PythonSystem.h"

#include "../EffectLib/EffectManager.h"
#include "../EffectLib/ParticleSystemData.h"
#include "../EterLib/Camera.h"

float CInstanceBase::ms_fDustGap;
float CInstanceBase::ms_fHorseDustGap;
uint32_t CInstanceBase::ms_adwCRCAffectEffect[CInstanceBase::EFFECT_NUM];
std::string CInstanceBase::ms_astAffectEffectAttachBone[EFFECT_NUM];

#define BYTE_COLOR_TO_D3DX_COLOR(r, g, b) D3DXCOLOR(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, 1.0f)

D3DXCOLOR g_akD3DXClrTitle[CInstanceBase::TITLE_NUM];
D3DXCOLOR g_akD3DXClrName[CInstanceBase::NAMECOLOR_NUM];

std::map<uint8_t, std::vector<std::string>> g_TitleNameMap;
std::set<uint32_t> g_setPVPReadyKey;
std::set<uint32_t> g_setPVPKeys;
std::set<uint32_t> g_setGVGKeys;
std::set<uint32_t> g_setDUELKey;

bool g_isEmpireNameMode = false;

void CInstanceBase::SetEmpireNameMode(bool isEnable)
{
	g_isEmpireNameMode = isEnable;

	g_akD3DXClrName[NAMECOLOR_MOB] = isEnable ? g_akD3DXClrName[NAMECOLOR_EMPIRE_MOB] : g_akD3DXClrName[NAMECOLOR_NORMAL_MOB];
	g_akD3DXClrName[NAMECOLOR_NPC] = isEnable ? g_akD3DXClrName[NAMECOLOR_EMPIRE_NPC] : g_akD3DXClrName[NAMECOLOR_NORMAL_NPC];
	g_akD3DXClrName[NAMECOLOR_PC] = g_akD3DXClrName[isEnable ? NAMECOLOR_NORMAL_PC : NAMECOLOR_NORMAL_PC];
	
	for (uint32_t uEmpire = isEnable ? 1 : 0; uEmpire < EMPIRE_NUM; ++uEmpire)
	{
		g_akD3DXClrName[NAMECOLOR_PC + uEmpire] = g_akD3DXClrName[isEnable ? (NAMECOLOR_EMPIRE_PC + uEmpire) : NAMECOLOR_NORMAL_PC];
	}
}

const D3DXCOLOR& CInstanceBase::GetIndexedNameColor(uint32_t eNameColor)
{
	static const D3DXCOLOR s_kD3DXClrNameDefault(0xffffffff);
	return eNameColor < NAMECOLOR_NUM ? g_akD3DXClrName[eNameColor] : s_kD3DXClrNameDefault;
}

void CInstanceBase::AddDamageEffect(DWORD damage, BYTE flag, bool bSelf, bool bTarget)
{
	if (IsAffect(AFFECT_EUNHYEONG) && (IsAffect(AFFECT_POISON) || IsAffect(AFFECT_FIRE)) && bTarget)
	{
		return;
	}

	if (auto& pythonSystem = CPythonSystem::Instance(); pythonSystem.IsShowDamage())
	{
		m_DamageQueue.emplace(damage, flag, bSelf, bTarget);
	}
}

bool CInstanceBase::ProcessDamage()
{
	if (m_DamageQueue.empty())
	{
		return false;
	}

	const auto& currentDamage = m_DamageQueue.front();
	m_DamageQueue.pop();

	DWORD damageAmount = currentDamage.damage;
	BYTE damageFlag = currentDamage.flag;
	BOOL isSelfDamage = currentDamage.bSelf;
	BOOL isTargetDamage = currentDamage.bTarget;

	auto* currentCamera = CCameraManager::Instance().GetCurrentCamera();
	float cameraAngle = GetDegreeFromPosition2(currentCamera->GetTarget().x, currentCamera->GetTarget().y, currentCamera->GetEye().x, currentCamera->GetEye().y);

	constexpr uint32_t fontWidth = 30;
	
	auto& effectManager = CEffectManager::Instance();

	D3DXVECTOR3 position = m_GraphicThingInstance.GetPosition();
	position.z += static_cast<float>(m_GraphicThingInstance.GetHeight());

	D3DXVECTOR3 rotation = {0.0f, 0.0f, cameraAngle};

	if (damageFlag & (DAMAGE_DODGE | DAMAGE_BLOCK))
	{
		effectManager.CreateEffect(ms_adwCRCAffectEffect[isSelfDamage ? EFFECT_DAMAGE_MISS : EFFECT_DAMAGE_TARGETMISS], position, rotation);
		return true;
	}

	std::string damageType;
	uint32_t effectCRCId = 0;

	if (isSelfDamage)
	{
		damageType = "damage_";
		effectCRCId = m_bDamageEffectType == 0 ? EFFECT_DAMAGE_SELFDAMAGE : EFFECT_DAMAGE_SELFDAMAGE2;
		m_bDamageEffectType = !m_bDamageEffectType;
	}
	else
	{
		damageType = isTargetDamage ? "target_" : "nontarget_";
		effectCRCId = isTargetDamage ? EFFECT_DAMAGE_TARGET : EFFECT_DAMAGE_NOT_TARGET;
	}

	uint32_t index = 0;
	uint32_t number = 0;
	std::vector<std::string> texturePaths;

	while (damageAmount > 0)
	{
		if (index > 7)
		{
			TraceError("ProcessDamage error: index out of range");
			break;
		}

		number = damageAmount % 10;
		damageAmount /= 10;

		auto numBuf = std::format("{}.dds", number);
		texturePaths.emplace_back(std::format("d:/ymir work/effect/affect/damagevalue/{}{}", damageType, numBuf));

		effectManager.SetEffectTextures(ms_adwCRCAffectEffect[effectCRCId], texturePaths);

		D3DXMATRIX transformationMatrix, translationMatrix;
		D3DXMatrixIdentity(&transformationMatrix);
		transformationMatrix._41 = position.x;
		transformationMatrix._42 = position.y;
		transformationMatrix._43 = position.z;
		D3DXMatrixTranslation(&transformationMatrix, position.x, position.y, position.z);
		D3DXMatrixMultiply(&transformationMatrix, &currentCamera->GetInverseViewMatrix(), &transformationMatrix);
		D3DXMatrixTranslation(&translationMatrix, fontWidth * index, 0, 0);
		translationMatrix._41 = -translationMatrix._41;
		transformationMatrix = translationMatrix * transformationMatrix;
		D3DXMatrixMultiply(&transformationMatrix, &currentCamera->GetViewMatrix(), &transformationMatrix);

		effectManager.CreateEffect(ms_adwCRCAffectEffect[effectCRCId], D3DXVECTOR3(transformationMatrix._41, transformationMatrix._42, transformationMatrix._43), rotation);

		texturePaths.clear();
		++index;
	}

	return true;
}

void CInstanceBase::AttachSpecialEffect(uint32_t effect)
{
	AttachEffect(effect);
}

void CInstanceBase::LevelUp()
{
	AttachEffect(EFFECT_LEVELUP);
}

void CInstanceBase::SkillUp()
{
	AttachEffect(EFFECT_SKILLUP);
}

void CInstanceBase::CreateSpecialEffect(uint32_t iEffectIndex)
{
	const D3DXMATRIX& globalTransformMatrix = m_GraphicThingInstance.GetTransform();

	uint32_t emptyEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	uint32_t effectCRC = ms_adwCRCAffectEffect[iEffectIndex];
	CEffectManager::Instance().CreateEffectInstance(emptyEffectIndex, effectCRC);
	CEffectManager::Instance().SelectEffectInstance(emptyEffectIndex);
	CEffectManager::Instance().SetEffectInstanceGlobalMatrix(globalTransformMatrix);
}

void CInstanceBase::EffectContainerDestroy()
{
	auto& dictionaryEffectID = __EffectContainer_GetDict();

	for (auto& effectPair : dictionaryEffectID)
	{
		DetachEffect(effectPair.second);
	}

	dictionaryEffectID.clear();
}

void CInstanceBase::EffectContainerInitialize()
{
	auto& dictionaryEffectID = __EffectContainer_GetDict();
	dictionaryEffectID.clear();
}

CInstanceBase::SEffectContainer::Dict& CInstanceBase::__EffectContainer_GetDict()
{
	return m_kEffectContainer.m_kDct_dwEftID;
}

uint32_t CInstanceBase::EffectContainerAttachEffect(uint32_t effectKey)
{
	auto& dictionaryEffectID = __EffectContainer_GetDict();
	auto [iterator, inserted] = dictionaryEffectID.try_emplace(effectKey, 0);

	if (!inserted)
	{
		return 0;
	}

	uint32_t effectID = AttachEffect(effectKey);
	iterator->second = effectID;
	return effectID;
}

void CInstanceBase::EffectContainerDetachEffect(uint32_t effectKey)
{
	auto& dictionaryEffectID = __EffectContainer_GetDict();

	if (auto it = dictionaryEffectID.find(effectKey); it != dictionaryEffectID.end())
	{
		DetachEffect(it->second);
		dictionaryEffectID.erase(it);
	}
}

void CInstanceBase::AttachEmpireEffect(uint32_t eEmpire)
{
#if defined(ENABLE_ATTACH_EMPIRE_EFFECT)
	if (!__IsExistMainInstance())
	{
		return;
	}

	CInstanceBase* pkInstMain = __GetMainInstancePtr();

	if (IsWarp())
	{
		return;
	}

	if (IsObject())
	{
		return;
	}

	if (IsFlag())
	{
		return;
	}

	if (IsResource())
	{
		return;
	}

	if (IsInvisibility())
	{
		return;
	}

	if (!pkInstMain->IsGameMaster() && pkInstMain->IsSameEmpire(*this))
	{
		return;
	}

	if (!pkInstMain->IsGameMaster())
	{
		if (pkInstMain->IsSameEmpire(*this))
		{
			return;
		}

		if (IsInvisibility())
		{
			return;
		}
	}

	__EffectContainer_AttachEffect(EFFECT_EMPIRE + eEmpire);
#endif
}

void CInstanceBase::AttachSelectEffect()
{
	EffectContainerAttachEffect(EFFECT_SELECT);
}

void CInstanceBase::DetachSelectEffect()
{
	EffectContainerDetachEffect(EFFECT_SELECT);
}

void CInstanceBase::AttachTargetEffect()
{
	EffectContainerAttachEffect(EFFECT_TARGET);
}

void CInstanceBase::DetachTargetEffect()
{
	EffectContainerDetachEffect(EFFECT_TARGET);
}

void CInstanceBase::StoneSmokeInialize()
{
	stoneSmokeEffect.effectID = 0;
}

void CInstanceBase::StoneSmokeDestroy()
{
	if (stoneSmokeEffect.effectID == 0)
	{
		return;
	}

	DetachEffect(stoneSmokeEffect.effectID);
	stoneSmokeEffect.effectID = 0;
}

void CInstanceBase::StoneSmokeCreate(uint32_t effectSmokeID)
{
	stoneSmokeEffect.effectID = m_GraphicThingInstance.AttachSmokeEffect(effectSmokeID);
}

void CInstanceBase::SetAlpha(float alpha)
{
	SetBlendRenderingMode();
	SetAlphaValue(alpha);
}

bool CInstanceBase::UpdateDeleting()
{
	Update();
	Transform();

	auto& app = IAbstractApplication::GetSingleton();

	float newAlpha = GetAlphaValue() - (app.GetGlobalElapsedTime() * 1.5f);
	SetAlphaValue(newAlpha);

	return newAlpha >= 0.0f;
}

void CInstanceBase::DeleteBlendOut()
{
	SetBlendRenderingMode();
	SetAlphaValue(1.0f);
	DetachTextTail();

	auto& playerInstance = IAbstractPlayer::GetSingleton();
	playerInstance.NotifyDeletingCharacterInstance(GetVirtualID());
}

void CInstanceBase::ClearPVPKeySystem()
{
	g_setPVPReadyKey.clear();
	g_setPVPKeys.clear();
	g_setGVGKeys.clear();
	g_setDUELKey.clear();
}

void CInstanceBase::InsertPVPKey(uint32_t sourceVID, uint32_t destinationVID)
{
	auto pvpKey = GetPVPKey(sourceVID, destinationVID);
	g_setPVPKeys.emplace(pvpKey);
}

void CInstanceBase::InsertPVPReadyKey(uint32_t sourceVID, uint32_t destinationVID)
{
	auto pvpReadyKey = GetPVPKey(sourceVID, destinationVID);
	g_setPVPKeys.emplace(pvpReadyKey);
}

void CInstanceBase::RemovePVPKey(uint32_t sourceVID, uint32_t destinationVID)
{
	auto pvpKey = GetPVPKey(sourceVID, destinationVID);
	g_setPVPKeys.erase(pvpKey);
}

void CInstanceBase::InsertGVGKey(uint32_t srcGuildVID, uint32_t dstGuildVID)
{
	auto gvgKey = GetPVPKey(srcGuildVID, dstGuildVID);
	g_setGVGKeys.emplace(gvgKey);
}

void CInstanceBase::RemoveGVGKey(uint32_t srcGuildVID, uint32_t dstGuildVID)
{
	auto gvgKey = GetPVPKey(srcGuildVID, dstGuildVID);
	g_setGVGKeys.erase(gvgKey);
}

void CInstanceBase::InsertDUELKey(uint32_t sourceVID, uint32_t destinationVID)
{
	auto duelKey = GetPVPKey(sourceVID, destinationVID);
	g_setDUELKey.emplace(duelKey);
}

uint32_t CInstanceBase::GetPVPKey(uint32_t sourceVID, uint32_t destinationVID)
{
	if (sourceVID > destinationVID)
	{
		std::swap(sourceVID, destinationVID);
	}

	std::array<uint32_t, 2> srcArray = {sourceVID, destinationVID};
	const auto* ptr = reinterpret_cast<const uint8_t*>(srcArray.data());
	const auto* end = ptr + sizeof(srcArray);
	uint32_t hash = 0;

	while (ptr < end) 
	{
		hash *= 16777619;
		hash ^= *ptr++;
	}

	return hash;
}

bool CInstanceBase::FindPVPKey(uint32_t sourceVID, uint32_t destinationVID)
{
	return g_setPVPKeys.contains(GetPVPKey(sourceVID, destinationVID));
}

bool CInstanceBase::FindPVPReadyKey(uint32_t sourceVID, uint32_t destinationVID)
{
	return g_setPVPReadyKey.contains(GetPVPKey(sourceVID, destinationVID));
}

bool CInstanceBase::FindGVGKey(uint32_t srcGuildVID, uint32_t dstGuildVID)
{
	return g_setGVGKeys.contains(GetPVPKey(srcGuildVID, dstGuildVID));
}

bool CInstanceBase::FindDUELKey(uint32_t sourceVID, uint32_t destinationVID)
{
	return g_setDUELKey.contains(GetPVPKey(sourceVID, destinationVID));
}

bool CInstanceBase::IsPVPInstance(CInstanceBase& rkInstSel)
{
	auto sourceVID = GetVirtualID();
	auto destinationVID = rkInstSel.GetVirtualID();

	auto guildIDSrc = GetGuildID();
	auto guildIDDst = rkInstSel.GetGuildID();

	return GetDuelMode() || FindPVPKey(sourceVID, destinationVID) || FindGVGKey(guildIDSrc, guildIDDst);
}

const D3DXCOLOR& CInstanceBase::GetNameColor()
{
	return GetIndexedNameColor(GetNameColorIndex());
}

uint32_t CInstanceBase::GetNameColorIndex()
{
	if (IsPC())
	{
		if (m_isKiller)
		{
			return NAMECOLOR_PK;
		}

		if (ExistsMainInstance() && !IsMainInstance())
		{
			CInstanceBase* pkInstMain = GetMainInstancePtr();

			if (!pkInstMain)
			{
				TraceError("CInstanceBase::GetNameColorIndex - MainInstance is NULL");
				return NAMECOLOR_PC;
			}

			auto dwVIDMain = pkInstMain->GetVirtualID();
			auto dwVIDSelf = GetVirtualID();

			if (pkInstMain->GetDuelMode())
			{
				switch (pkInstMain->GetDuelMode())
				{
					case DUEL_CANNOTATTACK:
					{
						return NAMECOLOR_PC + GetEmpireID();
					}

					case DUEL_START:
					{
						return FindDUELKey(dwVIDMain, dwVIDSelf) ? NAMECOLOR_PVP : NAMECOLOR_PC + GetEmpireID();
					}
				}
			}

			if (pkInstMain->IsSameEmpire(*this))
			{
				if (FindPVPKey(dwVIDMain, dwVIDSelf))
				{
					return NAMECOLOR_PVP;
				}

				if (FindGVGKey(pkInstMain->GetGuildID(), GetGuildID()))
				{
					return NAMECOLOR_PVP;
				}
			}
			else
			{
				return NAMECOLOR_PVP;
			}
		}

		return IAbstractPlayer::GetSingleton().IsPartyMemberByVID(GetVirtualID()) ? NAMECOLOR_PARTY : NAMECOLOR_PC + GetEmpireID();
	}

	if (IsNPC())
	{
		return NAMECOLOR_NPC;
	}

	if (IsEnemy() || IsPoly())
	{
		return NAMECOLOR_MOB;
	}

	return static_cast<uint32_t>(D3DXCOLOR(0xffffffff));
}

const D3DXCOLOR& CInstanceBase::GetTitleColor()
{
	const uint32_t alignmentGrade = GetAlignmentGrade();
	static const D3DCOLOR titleColorDefault = 0xFFFFFFFF;

	if (alignmentGrade >= TITLE_NUM)
	{
		return titleColorDefault;
	}

	return g_akD3DXClrTitle[alignmentGrade];
}

struct ScalingInfo 
{
	float min;
	float max;
	float scaleRatio;
};

float CInstanceBase::ScaleTextTailHeight(float baseHeight, bool IsMounting)
{
	float scaledHeight = baseHeight;
	if (IsMounting)
	{
		std::vector<ScalingInfo> scaleTable = 
		{
			{0.0f, 150.0f, -0.8f},
			{150.0f, 175.0f, -0.7f},
			{175.0f, 220.0f, -0.6f}
		};

		for (const auto& entry : scaleTable) 
		{
			if (baseHeight > entry.min && baseHeight <= entry.max) 
			{
				if (entry.scaleRatio < 0) 
				{
					scaledHeight -= baseHeight * std::abs(entry.scaleRatio);
				}
				else 
				{
					scaledHeight += (baseHeight - entry.min) * entry.scaleRatio;
				}
				break;
			}
		}
	}

	return scaledHeight;
}


void CInstanceBase::AttachTextTail()
{
	const uint32_t virtualID = GetVirtualID();

	if (m_isTextTail)
	{
		TraceError("CInstanceBase::AttachTextTail - VID [%d] ALREADY EXISTS", virtualID);
		return;
	}

	m_isTextTail = true;
	float fBaseHeight = GetBaseHeight();
	float fTextTailHeight = ScaleTextTailHeight(fBaseHeight, IsMountingHorse());

	static D3DXCOLOR s_kD3DXClrTextTail = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	CPythonTextTail::Instance().RegisterCharacterTextTail(m_dwGuildID, virtualID, s_kD3DXClrTextTail, fTextTailHeight);

	if (m_dwLevel)
	{
		UpdateTextTailLevel(m_dwLevel);
	}
}

void CInstanceBase::DetachTextTail()
{
	if (!m_isTextTail)
	{
		return;
	}

	m_isTextTail = false;
	CPythonTextTail::Instance().DeleteCharacterTextTail(GetVirtualID());
}

void CInstanceBase::UpdateTextTailLevel(uint32_t level)
{
	static const D3DXCOLOR kLevelColor = D3DXCOLOR(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);
	m_dwLevel = level;

	std::string szText;

	if (IsGameMaster())
	{
		szText = "[Mistrz Gry] ";
	}
	else
	{
		szText = "Poz. " + std::to_string(level) + " ";
	}

	CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText.c_str(), kLevelColor);
}

void CInstanceBase::RefreshTextTail()
{
	auto& pythonTextTail = CPythonTextTail::Instance();
	pythonTextTail.SetCharacterTextTailColor(GetVirtualID(), GetNameColor());
	const int32_t alignmentGrade = GetAlignmentGrade();

	if (alignmentGrade != TITLE_NONE)
	{
		std::map<uint8_t, std::vector<std::string>>::const_iterator itor = g_TitleNameMap.find(alignmentGrade);

		if (g_TitleNameMap.end() != itor)
		{
			const BYTE bRace = GetJob();
			const BYTE bSex = RaceToSex(bRace);
			const std::string& c_rstrTitleName = itor->second.at(bSex);

			CPythonTextTail::Instance().AttachTitle(GetVirtualID(), c_rstrTitleName.c_str(), GetTitleColor());
		}
	}
	else
	{
		pythonTextTail.DetachTitle(GetVirtualID());
	}
}

void CInstanceBase::RefreshTextTailTitle()
{
	RefreshTextTail();
}

void CInstanceBase::ClearAffectFlagContainer()
{
	affectFlagContainer.Clear();
}

void CInstanceBase::ClearAffects()
{
	if (IsStone())
	{
		StoneSmokeDestroy();
	}
	else
	{
		std::fill(std::begin(m_adwCRCAffectEffect), std::end(m_adwCRCAffectEffect), 0);

		for (auto effectID : m_adwCRCAffectEffect)
		{
			DetachEffect(effectID);
		}

		ClearAffectFlagContainer();
	}

	m_GraphicThingInstance.__OnClearAffects();
}

void CInstanceBase::SetNormalAffectFlagContainer(const CAffectFlagContainer& newAffectFlagContainer)
{
	for (int32_t index = 0; index < CAffectFlagContainer::BIT_SIZE; ++index)
	{
		bool isCurrentlySet = affectFlagContainer.IsSet(index);
		bool isNewSet = newAffectFlagContainer.IsSet(index);

		if (isCurrentlySet != isNewSet)
		{
			SetAffect(index, isNewSet);

			if (isNewSet)
			{
				m_GraphicThingInstance.__OnSetAffect(index);
			}
			else
			{
				m_GraphicThingInstance.__OnResetAffect(index);
			}
		}
	}

	affectFlagContainer = newAffectFlagContainer;
}

void CInstanceBase::SetStoneSmokeFlagContainer(const CAffectFlagContainer& affectFlagContainer)
{
	this->affectFlagContainer = affectFlagContainer;

	uint32_t effectSmokeID = 0;

	if (this->affectFlagContainer.IsSet(STONE_SMOKE8))
	{
		effectSmokeID = 3;
	}
	else if (this->affectFlagContainer.IsSet(STONE_SMOKE7) || this->affectFlagContainer.IsSet(STONE_SMOKE6) || this->affectFlagContainer.IsSet(STONE_SMOKE5))
	{
		effectSmokeID = 2;
	}
	else if (this->affectFlagContainer.IsSet(STONE_SMOKE4) || this->affectFlagContainer.IsSet(STONE_SMOKE3) || this->affectFlagContainer.IsSet(STONE_SMOKE2))
	{
		effectSmokeID = 1;
	}

	StoneSmokeDestroy();

	if (effectSmokeID != 0)
	{
		StoneSmokeCreate(effectSmokeID);
	}
}

void CInstanceBase::SetAffectFlagContainer(const CAffectFlagContainer& newAffectFlagContainer)
{
	if (IsBuilding())
	{
		return;		
	}
	else if (IsStone())
	{
		SetStoneSmokeFlagContainer(newAffectFlagContainer);
	}
	else
	{
		SetNormalAffectFlagContainer(newAffectFlagContainer);
		UpdateAttachingEffectVisibility(newAffectFlagContainer);
	}
}

void CInstanceBase::UpdateAttachingEffectVisibility(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	if (c_rkAffectFlagContainer.IsSet(AFFECT_REVIVE_INVISIBILITY) || c_rkAffectFlagContainer.IsSet(AFFECT_EUNHYEONG))
	{
		if (IsMainInstance())
		{
			m_GraphicThingInstance.ShowAllAttachingEffect();
		}
		else
		{
			m_GraphicThingInstance.HideAllAttachingEffect();
		}
	}
}

void CInstanceBase::SCRIPT_SetAffect(uint32_t eAffect, bool isVisible)
{
	SetAffect(eAffect, isVisible);
}

void CInstanceBase::__SetReviveInvisibilityAffect(bool isVisible)
{
	if (isVisible && !IsWearingDress())
	{
		if (IsMainInstance() || MainCanSeeHiddenThing())
		{
			m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
		}
		else
		{
			m_GraphicThingInstance.BlendAlphaValue(0.0f, 1.0f);
		}
	}
	else
	{
		if (!IsAffect(AFFECT_EUNHYEONG))
		{
			m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
			m_GraphicThingInstance.ShowAllAttachingEffect();
		}
	}
}

void CInstanceBase::__Assassin_SetEunhyeongAffect(bool isVisible)
{
	if (isVisible && !IsWearingDress())
	{
		if (IsMainInstance() || MainCanSeeHiddenThing())
		{
			m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
		}
		else
		{
			m_GraphicThingInstance.BlendAlphaValue(0.0f, 1.0f);
			m_GraphicThingInstance.HideAllAttachingEffect();
		}
	}
	else
	{

		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
		m_GraphicThingInstance.ShowAllAttachingEffect();
	}
}

void CInstanceBase::SetParalysisShaman(bool isParalysis)
{
	m_GraphicThingInstance.SetParalysis(isParalysis);
}

void CInstanceBase::SetWarriorGeomgyeongEffectVisibility(bool isVisible)
{
	if (isVisible && !IsWearingDress())
	{
		if (m_kWarrior.m_dwGeomgyeongEffect != 0)
		{
			DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);
		}

		m_GraphicThingInstance.SetReachScale(1.5f);
		m_kWarrior.m_dwGeomgyeongEffect = AttachEffect(EFFECT_WEAPON + (m_GraphicThingInstance.IsTwoHandMode() ? WEAPON_TWOHAND : WEAPON_ONEHAND));
	}
	else if (!isVisible)
	{
		m_GraphicThingInstance.SetReachScale(1.0f);
		DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);
		m_kWarrior.m_dwGeomgyeongEffect = 0;
	}
}

void CInstanceBase::SetAffect(uint32_t eAffect, bool isVisible)
{
	if (eAffect >= AFFECT_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetAffect(eAffect:%d < AFFECT_NUM:%d, isVisible=%d)", GetVirtualID(), eAffect, AFFECT_NUM, isVisible);
		return;
	}

	switch (eAffect)
	{
		case AFFECT_YMIR:
		{
			if (IsAffect(AFFECT_INVISIBILITY))
			{
				return;
			}
			break;
		}

		case AFFECT_CHEONGEUN:
		{
			m_GraphicThingInstance.SetResistFallen(isVisible);
			break;
		}

		case AFFECT_GEOMGYEONG:
		{
			SetWarriorGeomgyeongEffectVisibility(isVisible);
			return;
		}

		case AFFECT_REVIVE_INVISIBILITY:
		{
			__SetReviveInvisibilityAffect(isVisible);
			break;
		}

		case AFFECT_EUNHYEONG:
		{
			__Assassin_SetEunhyeongAffect(isVisible);
			break;
		}

		case AFFECT_GYEONGGONG:
		case AFFECT_KWAESOK:
		{
			if (isVisible && !IsWalking())
			{
				return;
			}
			break;
		}

		case AFFECT_INVISIBILITY:
		{
			if (isVisible)
			{
				EffectContainerDestroy();
				DetachTextTail();
				m_GraphicThingInstance.HideAllAttachingEffect();
			}
			else
			{
				m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
				AttachTextTail();
				RefreshTextTail();
				m_GraphicThingInstance.ShowAllAttachingEffect();
			}
			return;
		}

		case AFFECT_STUN:
		{
			m_GraphicThingInstance.SetSleep(isVisible);
			break;
		}
	}

	HandleEffectVisibility(eAffect, isVisible);
}

void CInstanceBase::HandleEffectVisibility(uint32_t eAffect, bool isVisible)
{
	if (isVisible)
	{
		if (!m_adwCRCAffectEffect[eAffect])
		{
			m_adwCRCAffectEffect[eAffect] = AttachEffect(EFFECT_AFFECT + eAffect);
		}
	}
	else
	{
		if (m_adwCRCAffectEffect[eAffect])
		{
			DetachEffect(m_adwCRCAffectEffect[eAffect]);
			m_adwCRCAffectEffect[eAffect] = 0;
		}
	}
}

bool CInstanceBase::IsPossibleEmoticon() const
{
	CEffectManager& rkEftMgr = CEffectManager::Instance();
	for (DWORD eEmoticon = 0; eEmoticon < EMOTICON_NUM; eEmoticon++)
	{
		DWORD effectID = ms_adwCRCAffectEffect[EFFECT_EMOTICON + eEmoticon];
		if (effectID && rkEftMgr.IsAliveEffect(effectID))
		{
			return false;
		}
	}

	auto now = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastEmoticonTime).count() < 1000)
	{
		return false;
	}

	return true;
}

void CInstanceBase::SetFishEmoticon()
{
	SetEmoticon(EMOTICON_FISH);
}

void CInstanceBase::SetEmoticon(uint32_t eEmoticon)
{
	if (eEmoticon >= EMOTICON_NUM || !IsPossibleEmoticon())
	{
		return;
	}

	float baseHeight = GetBaseHeight();
	bool isMounting = IsMountingHorse();
	float adjustmentFactor = 180.0f + (CPythonTextTail::Instance().IsChatTextTail(GetVirtualID()) ? (isMounting ? 11.0f : 30.0f) : 0.0f);
	float finalHeight = ScaleTextTailHeight(baseHeight, isMounting) + adjustmentFactor;

	D3DXVECTOR3 v3Pos = D3DXVECTOR3(0, 0, finalHeight);
	m_GraphicThingInstance.AttachEffectByID(0, nullptr, ms_adwCRCAffectEffect[EFFECT_EMOTICON + eEmoticon], &v3Pos);
	m_lastEmoticonTime = std::chrono::steady_clock::now();
}

void CInstanceBase::SetDustGap(float fDustGap)
{
	ms_fDustGap = fDustGap;
}

void CInstanceBase::SetHorseDustGap(float fDustGap)
{
	ms_fHorseDustGap = fDustGap;
}

void CInstanceBase::DetachEffect(uint32_t dwEID)
{
	m_GraphicThingInstance.DettachEffect(dwEID);
}

uint32_t CInstanceBase::AttachEffect(uint32_t eEftType)
{
	if (IsAffect(AFFECT_INVISIBILITY) || eEftType >= EFFECT_NUM) 
	{
		return 0;
	}

	const auto& boneName = ms_astAffectEffectAttachBone[eEftType];

	if (boneName.empty()) 
	{
		return m_GraphicThingInstance.AttachEffectByID(0, nullptr, ms_adwCRCAffectEffect[eEftType]);
	}

	const char* c_szBoneName = nullptr;

	if (boneName == "PART_WEAPON" || boneName == "PART_WEAPON_LEFT") 
	{
		auto partType = boneName == "PART_WEAPON" ? CRaceData::PART_WEAPON : CRaceData::PART_WEAPON_LEFT;

		if (!m_GraphicThingInstance.GetAttachingBoneName(partType, &c_szBoneName))
		{
			return 0;
		}
	}
	else 
	{
		c_szBoneName = boneName.c_str();
	}

	return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType]);
}

bool CInstanceBase::RegisterEffect(uint32_t effectType, const char* effectAttachBoneName, const char* effectFileName, bool shouldCache)
{
	if (effectType >= EFFECT_NUM)
	{
		return false;
	}

	ms_astAffectEffectAttachBone[effectType] = std::string(effectAttachBoneName);

	auto& effectCRC = ms_adwCRCAffectEffect[effectType];

	if (!CEffectManager::Instance().RegisterEffect2(effectFileName, &effectCRC, shouldCache))
	{
		TraceError("CInstanceBase::RegisterEffect(effectType=%d, effectAttachBoneName=%s, effectFileName=%s, shouldCache=%d) - Error", effectType, effectAttachBoneName, effectFileName, shouldCache);
		effectCRC = 0;
		return false;
	}

	return true;
}

void CInstanceBase::RegisterTitleName(const int iIndex, const char* c_szTitleNameF, const char* c_szTitleNameM)
{
	std::vector<std::string> vecTitleName{ c_szTitleNameF, c_szTitleNameM };
	g_TitleNameMap.insert(make_pair(iIndex, vecTitleName));
}

static D3DXCOLOR RGBToD3DXColor(uint32_t red, uint32_t green, uint32_t blue)
{
	return D3DXCOLOR(static_cast<float>(red) / 255.0f, static_cast<float>(green) / 255.0f, static_cast<float>(blue) / 255.0f, 1.0f);
}

bool CInstanceBase::RegisterNameColor(uint32_t index, uint32_t red, uint32_t green, uint32_t blue)
{
	if (index >= NAMECOLOR_NUM)
	{
		return false;
	}

	g_akD3DXClrName[index] = RGBToD3DXColor(red, green, blue);
	return true;
}

bool CInstanceBase::RegisterTitleColor(uint32_t index, uint32_t red, uint32_t green, uint32_t blue) 
{
	if (index >= TITLE_NUM) 
	{
		return false;
	}

	g_akD3DXClrTitle[index] = RGBToD3DXColor(red, green, blue);
	return true;
}
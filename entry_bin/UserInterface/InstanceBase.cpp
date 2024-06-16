#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#include "AbstractApplication.h"
#include "packet.h"
#include "PythonSystem.h"
#include "PythonTextTail.h"

#include "../eterlib/StateManager.h"
#include "../gamelib/ItemManager.h"

#include "../gamelib/RaceManager.h"

BOOL HAIR_COLOR_ENABLE = TRUE;
BOOL USE_ARMOR_SPECULAR = FALSE;
BOOL RIDE_HORSE_ENABLE = FALSE;

const float c_fDefaultRotationSpeed = 1200.0f;
const float c_fDefaultHorseRotationSpeed = 300.0f;

bool IsWall(uint16_t race)
{
	return race >= 14201 && race <= 14204;
}

CInstanceBase::SHORSE::SHORSE()
{
	Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
	assert(actorPointer == nullptr);
}

void CInstanceBase::SHORSE::Initialize()
{
	isCurrentlyMounting = false;
	actorPointer = nullptr;
}

void CInstanceBase::SHORSE::SetAttackSpeed(uint32_t attackSpeed)
{
	if (!IsMounting())
	{
		return;
	}

	CActorInstance& actorReference = GetActorRef();
	actorReference.SetAttackSpeed(static_cast<float>(attackSpeed) / 100.0f);
}

void CInstanceBase::SHORSE::SetMoveSpeed(uint32_t moveSpeed)
{	
	if (!IsMounting())
	{
		return;
	}

	CActorInstance& actorReference = GetActorRef();
	actorReference.SetMoveSpeed(static_cast<float>(moveSpeed) / 100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition& pixelPosition, uint32_t raceType, uint32_t hitEffectType)
{
	assert(nullptr == actorPointer && "CInstanceBase::SHORSE::Create - ALREADY MOUNT");

	actorPointer = std::make_unique<CActorInstance>();

	CActorInstance& actorReference = GetActorRef();

	actorReference.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

	if (!actorReference.SetRace(raceType))
	{
		actorPointer.reset();
		return;
	}

	actorReference.SetShape(0);
	actorReference.SetAlphaValue(0.0f);
	actorReference.BlendAlphaValue(1.0f, 0.5f);
	actorReference.SetMoveSpeed(1.0f);
	actorReference.SetAttackSpeed(1.0f);
	actorReference.SetMotionMode(CRaceMotionData::MODE_GENERAL);
	actorReference.Stop();
	actorReference.RefreshActorInstance();
	actorReference.SetCurPixelPosition(pixelPosition);

	isCurrentlyMounting = true;
}

void CInstanceBase::SHORSE::Destroy()
{
	actorPointer.reset();
	Initialize();
}

CActorInstance& CInstanceBase::SHORSE::GetActorRef() const
{
	assert(actorPointer != nullptr && "CInstanceBase::SHORSE::GetActorRef - ACTOR POINTER IS NULLPTR");
	return *actorPointer;
}

CActorInstance* CInstanceBase::SHORSE::GetActorPtr() const
{
	return actorPointer.get();
}

uint32_t CInstanceBase::SHORSE::GetLevel() const
{
	if (actorPointer)
	{
		uint32_t mount = actorPointer->GetRace();

		switch (mount)
		{
			case 20101:
			case 20102:
			case 20103:
			{
				return 1;
			}

			case 20104:
			case 20105:
			case 20106:
			{
				return 2;
			}

			case 20107:
			case 20108:
			case 20109:
			case 20110:
			case 20111: 
			case 20112:
			case 20113:
			case 20114:
			case 20115:
			case 20116:
			case 20117:
			case 20118:
			case 20120:
			case 20121:
			case 20122:
			case 20123:
			case 20124:
			case 20125:
				return 3;

			case 20119:
			case 20219:
			case 20220:
			case 20221:
			case 20222:
			{
				return 2;
			}
		}
		{
			if ((20205 <= mount && 20208 >= mount) || (20214 == mount) || (20217 == mount))
			{
				return 2;
			}

			if ((20209 <= mount && 20212 >= mount) || (20215 == mount) || (20218 == mount) || (20220 == mount))
			{
				return 3;
			}
		}
	}

	return 0;
}

bool CInstanceBase::SHORSE::IsNewMount() const
{
	if (!actorPointer)
	{
		return false;
	}

	uint32_t mount = actorPointer->GetRace();

	if ((20205 <= mount && 20208 >= mount) || (20214 == mount) || (20217 == mount))
	{
		return true;
	}

	if ((20209 <= mount && 20212 >= mount) || (20215 == mount) || (20218 == mount) || (20220 == mount))
	{
		return true;
	}

	return false;
}

bool CInstanceBase::SHORSE::CanUseSkill() const
{
	if (IsMounting())
	{
		return GetLevel() > 2;
	}

	return true;
}

bool CInstanceBase::SHORSE::CanAttack() const
{
	if (IsMounting())
	{
		if (GetLevel() <= 1)
		{
			return false;
		}
	}

	return true;
}
			
bool CInstanceBase::SHORSE::IsMounting() const
{
	return isCurrentlyMounting;
}

void CInstanceBase::SHORSE::Deform() const
{
	if (!IsMounting())
	{
		return;
	}

	CActorInstance& actorReference = GetActorRef();
	actorReference.INSTANCEBASE_Deform();
}

void CInstanceBase::SHORSE::Render() const
{
	if (!IsMounting())
	{
		return;
	}

	CActorInstance& actorReference = GetActorRef();
	actorReference.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
	if (!IsMountingHorse())
	{
		return;
	}

	m_kHorse.actorPointer->AttachModelInstance(CRaceData::PART_MAIN, "saddle", m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::__DetachHorseSaddle()
{
	if (!IsMountingHorse())
	{
		return;
	}

	m_kHorse.actorPointer->DetachModelInstance(CRaceData::PART_MAIN, m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::BlockMovement()
{
	m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

///////////////////////////////////////////////////////////////////////////////////

bool __ArmorVnumToShape(int iVnum, DWORD * pdwShape)
{
	*pdwShape = iVnum;

	/////////////////////////////////////////

	if (0 == iVnum || 1 == iVnum)
		return false;

	if (!USE_ARMOR_SPECULAR)
		return false;

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
		return false;

	enum
	{
		SHAPE_VALUE_SLOT_INDEX = 3,
	};

	*pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

	return true;
}

class CActorInstanceBackground : public IBackground
{
	public:
		CActorInstanceBackground() = default;
		virtual ~CActorInstanceBackground() override = default;

		bool IsBlock(int x, int y) override
		{
			auto& rkBG = CPythonBackground::Instance();
			return rkBG.isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
		}
};

static CActorInstanceBackground gs_kActorInstBG;

bool CInstanceBase::LessRenderOrder(CInstanceBase* otherInstance)
{
	int32_t currentAlphaFlag = (GetAlphaValue() < 1.0f) ? 1 : 0;
	int32_t otherAlphaFlag = (otherInstance->GetAlphaValue() < 1.0f) ? 1 : 0;

	if (currentAlphaFlag < otherAlphaFlag)
	{
		return true;
	}

	if (currentAlphaFlag > otherAlphaFlag)
	{
		return false;
	}

	if (GetRace() < otherInstance->GetRace())
	{
		return true;
	}

	if (GetRace() > otherInstance->GetRace())
	{
		return false;
	}

	if (GetShape() < otherInstance->GetShape())
	{
		return true;
	}

	if (GetShape() > otherInstance->GetShape())
	{
		return false;
	}

	uint32_t currentLODLevel = __LessRenderOrder_GetLODLevel();
	uint32_t otherLODLevel = otherInstance->__LessRenderOrder_GetLODLevel();

	if (currentLODLevel < otherLODLevel)
	{
		return true;
	}

	if (currentLODLevel > otherLODLevel)
	{
		return false;
	}

	if (m_awPart[CRaceData::PART_WEAPON] < otherInstance->m_awPart[CRaceData::PART_WEAPON])
	{
		return true;
	}

	return false;
}

uint32_t CInstanceBase::__LessRenderOrder_GetLODLevel()
{
	auto* lodController = m_GraphicThingInstance.GetLODControllerPointer(0);

	if (!lodController)
	{
		return 0;
	}

	return lodController->GetLODLevel();
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition& pixelPosition, float* height)
{
	long waterHeight;

	if (!CPythonBackground::Instance().GetWaterHeight(static_cast<int32_t>(pixelPosition.x), static_cast<int32_t>(pixelPosition.y), &waterHeight))
	{
		return false;
	}

	*height = static_cast<float>(waterHeight);

	return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos)
{
	return CPythonBackground::Instance().isAttrOn(c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

DWORD CInstanceBase::ms_dwUpdateCounter=0;
DWORD CInstanceBase::ms_dwRenderCounter=0;
DWORD CInstanceBase::ms_dwDeformCounter=0;

CDynamicPool<CInstanceBase> CInstanceBase::ms_kPool;

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

bool CInstanceBase::__IsInDustRange()
{
	if (!ExistsMainInstance())
	{
		return false;
	}

	auto* mainInstance = GetMainInstancePtr();
	float distance = NEW_GetDistanceFromDestInstance(*mainInstance);
	float dustRange = IsPC() ? PC_DUST_RANGE : NPC_DUST_RANGE;

	return distance <= dustRange;
}

void CInstanceBase::__EnableSkipCollision()
{
	if (IsMainInstance())
	{
		TraceError("CInstanceBase::__EnableSkipCollision - Cannot skip collision for the main instance!");
		return;
	}

	m_GraphicThingInstance.EnableSkipCollision();
}

void CInstanceBase::__DisableSkipCollision()
{
	m_GraphicThingInstance.DisableSkipCollision();
}

uint32_t CInstanceBase::__GetShadowMapColor(float x, float y)
{
	auto& background = CPythonBackground::Instance();
	return background.GetShadowMapColor(x, y);
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
	auto& background = CPythonBackground::Instance();
	return background.GetHeight(x, y);
}

bool CInstanceBase::IsInvisibility()
{
	if ((IsAffect(AFFECT_INVISIBILITY) || IsAffect(AFFECT_EUNHYEONG) || IsAffect(AFFECT_REVIVE_INVISIBILITY)))
	{
		return true;
	}

	return false;
}

bool CInstanceBase::IsParalysis()
{
	return m_GraphicThingInstance.IsParalysis();
}

bool CInstanceBase::IsGameMaster()
{
    return affectFlagContainer.IsSet(AFFECT_YMIR);
}

bool CInstanceBase::IsSameEmpire(CInstanceBase& otherInstance)
{
	return otherInstance.m_dwEmpireID == 0 || IsGameMaster() || otherInstance.IsGameMaster() || otherInstance.m_dwEmpireID == m_dwEmpireID;
}

uint32_t CInstanceBase::GetEmpireID() const
{
	return m_dwEmpireID;
}

uint32_t CInstanceBase::GetGuildID() const
{
	return m_dwGuildID;
}

int32_t CInstanceBase::GetAlignment() const
{
	return m_sAlignment;
}

uint32_t CInstanceBase::GetAlignmentGrade() const
{
	if (m_sAlignment >= 12000)
	{
		return 0;
	}

	if (m_sAlignment >= 8000)
	{
		return 1;
	}

	if (m_sAlignment >= 4000)
	{
		return 2;
	}

	if (m_sAlignment >= 1000)
	{
		return 3;
	}

	if (m_sAlignment >= 0)
	{
		return 4;
	}

	if (m_sAlignment > -4000)
	{
		return 5;
	}

	if (m_sAlignment > -8000)
	{
		return 6;
	}

	if (m_sAlignment > -12000)
	{
		return 7;
	}

	return 8;
}

int32_t CInstanceBase::GetAlignmentType() const
{
	uint32_t alignmentGrade = GetAlignmentGrade();

	if (alignmentGrade <= 3)
	{
		return ALIGNMENT_TYPE_WHITE;
	}

	if (alignmentGrade >= 5 && alignmentGrade <= 8)
	{
		return ALIGNMENT_TYPE_DARK;
	}

	return ALIGNMENT_TYPE_NORMAL;
}

uint8_t CInstanceBase::GetPKMode() const
{
	return m_byPKMode;
}

bool CInstanceBase::IsKiller() const
{
	return m_isKiller;
}

bool CInstanceBase::IsPartyMember() const
{
	return m_isPartyMember;
}

bool CInstanceBase::IsInSafe()
{
	const TPixelPosition& currentPixelPosition = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return CPythonBackground::Instance().isAttrOn(currentPixelPosition.x, currentPixelPosition.y, CTerrainImpl::ATTRIBUTE_BANPK);
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition& destinationPixelPosition)
{
	const TPixelPosition& sourcePixelPosition = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return SPixelPosition_CalculateDistanceSq3d(sourcePixelPosition, destinationPixelPosition);
}

void CInstanceBase::OnSelected()
{
	if (!IsStoneDoor() && !IsDead())
	{
		AttachSelectEffect();
	}
}

void CInstanceBase::OnUnselected()
{
	DetachSelectEffect();
}

void CInstanceBase::OnTargeted()
{
	if (!IsStoneDoor() && !IsDead())
	{
		AttachTargetEffect();
	}
}

void CInstanceBase::OnUntargeted()
{
	DetachTargetEffect();
}

void CInstanceBase::DestroySystem()
{
	ms_kPool.Clear();
}

void CInstanceBase::CreateSystem(uint32_t capacity)
{
	ms_kPool.Create(capacity);

	std::fill(std::begin(ms_adwCRCAffectEffect), std::end(ms_adwCRCAffectEffect), 0);

	constexpr float kDustGap_Min = 250.0f;
	constexpr float kDustGap_Max = 500.0f;

	ms_fDustGap = kDustGap_Min;
	ms_fHorseDustGap = kDustGap_Max;
}

CInstanceBase* CInstanceBase::New()
{
	return ms_kPool.Alloc();
}

void CInstanceBase::Delete(CInstanceBase* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CInstanceBase::SetMainInstance()
{
	auto& characterManager = CPythonCharacterManager::Instance();
	auto virtualID = GetVirtualID();
	characterManager.SetMainInstance(virtualID);
	m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase* CInstanceBase::GetMainInstancePtr() const
{
	auto& characterManager = CPythonCharacterManager::Instance();
	return characterManager.GetMainInstancePtr();
}

void CInstanceBase::ClearMainInstance()
{
	auto& characterManager = CPythonCharacterManager::Instance();
	characterManager.ClearMainInstance();
}

bool CInstanceBase::IsMainInstance() const
{
	return this == GetMainInstancePtr();
}

bool CInstanceBase::ExistsMainInstance() const
{
	return GetMainInstancePtr() != nullptr;
}

bool CInstanceBase::MainCanSeeHiddenThing() const
{
	return false;
}

float CInstanceBase::GetBowRange() const
{
	float baseRange = BASE_BOW_RANGE;

	if (IsMainInstance())
	{
		auto& player = IAbstractPlayer::GetSingleton();
		baseRange += static_cast<float>(player.GetStatus(POINT_BOW_DISTANCE));
	}

	return baseRange;
}

CInstanceBase* CInstanceBase::FindInstancePtr(uint32_t virtualID) const
{
	auto& characterManager = CPythonCharacterManager::Instance();
	return characterManager.GetInstancePtr(virtualID);
}

bool CInstanceBase::FindRaceType(uint32_t raceID, uint8_t* typePtr) const
{
	auto& nonPlayerManager = CPythonNonPlayer::Instance();
	return nonPlayerManager.GetInstanceType(raceID, typePtr);
}

bool CInstanceBase::Create(const SCreateData& createData)
{
	IAbstractApplication::GetSingleton().SkipRenderBuffering(300);

	SetInstanceType(createData.m_bType);

	if (!SetRace(createData.m_dwRace))
	{
		return false;
	}

	if (IsPC())
	{
		SetJob(createData.m_bJob);
	}

	SetVirtualID(createData.m_dwVID);

	if (createData.m_isMain)
	{
		SetMainInstance();
	}

	const bool isGuildWall = IsGuildWall();
	const bool isPC = IsPC();

	if (isGuildWall)
	{
		HandleGuildWall(createData);
	}
	else
	{
		SCRIPT_SetPixelPosition(float(createData.m_lPosX), float(createData.m_lPosY));
	}

	if (createData.m_dwMountVnum)
	{
		MountHorse(createData.m_dwMountVnum);
	}

	SetArmor(createData.m_dwArmor);

	if (IsPC() || (IsNPC() && createData.m_dwHair))
	{
		SetHair(createData.m_dwHair);
	}

	if (IsPC())
	{
		SetWeapon(createData.m_dwWeapon);
	}

	CreateSetName(createData);

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	m_dwLevel = (IsEnemy() && CPythonSystem::Instance().IsShowMobLevel()) ? CPythonNonPlayer::Instance().GetMonsterLevel(GetRace()) : createData.m_dwLevel;
#else
	m_dwLevel = createData.m_dwLevel;
#endif

    m_dwGuildID = createData.m_dwGuildID;
	m_dwEmpireID = createData.m_dwEmpireID;
	SetVirtualNumber(createData.m_dwRace);
	SetRotation(createData.m_fRot);
	SetAlignment(createData.m_sAlignment);
	SetPKMode(createData.m_byPKMode);
	SetMoveSpeed(createData.m_dwMovSpd);
	SetAttackSpeed(createData.m_dwAtkSpd);
	
	if (!IsWearingDress())
	{
		m_GraphicThingInstance.SetAlphaValue(0.0f);
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);
	}

	if (!IsGuildWall())
	{
		SetAffectFlagContainer(createData.m_kAffectFlags);
	}	

	AttachTextTail();
	RefreshTextTail();

	if (createData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN)
	{
		if (IsAffect(AFFECT_SPAWN))
		{
			AttachEffect(EFFECT_SPAWN_APPEAR);
		}

		Refresh(isPC ? CRaceMotionData::NAME_WAIT : CRaceMotionData::NAME_SPAWN, isPC);
	}
	else
	{
		Refresh(CRaceMotionData::NAME_WAIT, true);
	}

	AttachEmpireEffect(createData.m_dwEmpireID);
	RegisterBoundingSphere();

	if (createData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
	{
		m_GraphicThingInstance.DieEnd();
	}

	SetStateFlags(createData.m_dwStateFlags);
	m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);

	if (!isPC)
	{
		uint32_t dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(createData.m_dwRace);
		if (0 != dwBodyColor)
		{
			SetModulateRenderMode();
			SetAddColor(dwBodyColor);
		}
	}

	__AttachHorseSaddle();

	if (GUILD_SYMBOL_RACE == GetRace())
	{
		std::string strFileName = GetGuildSymbolFileName(m_dwGuildID);

		if (IsFile(strFileName.c_str()))
		{
			m_GraphicThingInstance.ChangeMaterial(strFileName.c_str());
		}
	}

	return true;
}

void CInstanceBase::HandleGuildWall(const SCreateData& createData)
{
	uint32_t center_x, center_y;
	createData.m_kAffectFlags.ConvertToPosition(&center_x, &center_y);

	float center_z = __GetBackgroundHeight(center_x, center_y);
	NEW_SetPixelPosition(TPixelPosition(static_cast<float>(createData.m_lPosX), static_cast<float>(createData.m_lPosY), center_z));
}

void CInstanceBase::CreateSetName(const SCreateData& createData)
{
	if (IsGoto() || IsWarp())
	{
		if (IsWarp())
		{
			CreateSetWarpName(createData);
		}
		else
		{
			SetNameString("", 0);
		}
		return;
	}

#if defined(WJ_SHOW_MOB_INFO)
	std::string name = createData.m_stName;

	if (IsEnemy())
	{
		auto& pythonNonPlayer = CPythonNonPlayer::Instance();

#if defined(ENABLE_SHOW_MOBAIFLAG)
		if (CPythonSystem::Instance().IsShowMobAIFlag() && pythonNonPlayer.IsAggressive(GetRace()))
		{
			name += "*";
		}
#endif
	}

	SetNameString(name.c_str(), name.length());
#else
	SetNameString(createData.m_stName.c_str(), createData.m_stName.length());
#endif
}

void CInstanceBase::CreateSetWarpName(const SCreateData& createData)
{
	const char* c_szName;

	if (CPythonNonPlayer::Instance().GetName(createData.m_dwRace, &c_szName))
	{
		std::string strName = c_szName;
		auto iFindingPos = strName.find_first_of(' ');

		if (iFindingPos != std::string::npos)
		{
			strName.resize(iFindingPos);
		}

		SetNameString(strName.c_str(), strName.length());
	}
	else
	{
		SetNameString(createData.m_stName.c_str(), createData.m_stName.length());
	}
}

void CInstanceBase::SetNameString(const char* c_szName, int len)
{
	m_stName.assign(c_szName, len);
}

bool CInstanceBase::SetRace(uint32_t newRace)
{
	m_dwRace = newRace;

	if (!m_GraphicThingInstance.SetRace(newRace))
		return false;

	if (!FindRaceType(m_dwRace, &m_eRaceType))
		m_eRaceType=CActorInstance::TYPE_PC;

	return true;
}

bool CInstanceBase::__IsChangableWeapon(int32_t weaponID) const
{	
	if (IsWearingDress())
	{
		static const std::vector<int32_t> allowedBouquets = {50201, 50202, 50203, 50204};
		return std::find(allowedBouquets.begin(), allowedBouquets.end(), weaponID) != allowedBouquets.end();
	}
	else
	{
		return true;
	}
}

bool CInstanceBase::IsWearingDress() const
{
	static constexpr int32_t WeddingDressShape = 201;
	return WeddingDressShape == m_eShape;
}

bool CInstanceBase::IsHoldingPickAxe() const
{
	static constexpr int32_t PickAxeStart = 29101;
	static constexpr int32_t PickAxeEnd = 29110;
	return m_awPart[CRaceData::PART_WEAPON] >= PickAxeStart && m_awPart[CRaceData::PART_WEAPON] <= PickAxeEnd;
}

bool CInstanceBase::IsNewMount()
{
	return m_kHorse.IsNewMount();
}

bool CInstanceBase::IsMountingHorse()
{
	return m_kHorse.IsMounting();
}

void CInstanceBase::MountHorse(UINT eRace)
{
	m_kHorse.Destroy();
	m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace, ms_adwCRCAffectEffect[EFFECT_HIT]);

	SetMotionMode(CRaceMotionData::MODE_HORSE);	
	SetRotationSpeed(c_fDefaultHorseRotationSpeed);

	m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
	m_GraphicThingInstance.Stop();
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::DismountHorse()
{
	m_kHorse.Destroy();
}

void CInstanceBase::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "Inst - UC %d, RC %d Pool - %d ", 
		ms_dwUpdateCounter, 
		ms_dwRenderCounter,
		ms_kPool.GetCapacity()
	);

	pstInfo->append(szInfo);
}

void CInstanceBase::ResetPerformanceCounter()
{
	ms_dwUpdateCounter=0;
	ms_dwRenderCounter=0;
	ms_dwDeformCounter=0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
	return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition& CInstanceBase::NEW_GetLastPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(FLOAT z)
{
	m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc)
{
	m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition& CInstanceBase::NEW_GetCurPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();	
}

const TPixelPosition& CInstanceBase::NEW_GetDstPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetSrcPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CInstanceBase::OnSyncing()
{
	m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
	m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
	m_GraphicThingInstance.__OnMoving();
}

void CInstanceBase::ChangeGuild(DWORD dwGuildID)
{
	m_dwGuildID=dwGuildID;

	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}

DWORD CInstanceBase::GetPart(CRaceData::EParts part)
{
	assert(part >= 0 && part < CRaceData::PART_MAX_NUM);
	return m_awPart[part];
}

DWORD CInstanceBase::GetShape()
{
	return m_eShape;
}

bool CInstanceBase::CanAct()
{
	return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
	return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill()
{
	if (IsPoly())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	if (!m_kHorse.CanUseSkill())
		return false;

	if (!m_GraphicThingInstance.CanUseSkill())
		return false;

	return true;
}

bool CInstanceBase::CanAttack()
{
	if (!m_kHorse.CanAttack())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;
	
	return m_GraphicThingInstance.CanAttack();
}



bool CInstanceBase::CanFishing()
{
	return m_GraphicThingInstance.CanFishing();
}


BOOL CInstanceBase::IsBowMode()
{
	return m_GraphicThingInstance.IsBowMode();
}

BOOL CInstanceBase::IsHandMode()
{
	return m_GraphicThingInstance.IsHandMode();
}

BOOL CInstanceBase::IsFishingMode()
{
	if (CRaceMotionData::MODE_FISHING == m_GraphicThingInstance.GetMotionMode())
		return true;

	return false;
}

BOOL CInstanceBase::IsFishing()
{
	return m_GraphicThingInstance.IsFishing();
}

BOOL CInstanceBase::IsDead()
{
	return m_GraphicThingInstance.IsDead();
}

BOOL CInstanceBase::IsStun()
{
	return m_GraphicThingInstance.IsStun();
}

BOOL CInstanceBase::IsSleep()
{
	return m_GraphicThingInstance.IsSleep();
}

bool CInstanceBase::IsMining()
{
	return m_GraphicThingInstance.IsMining();
}

BOOL CInstanceBase::__IsSyncing()
{
	return m_GraphicThingInstance.__IsSyncing();
}

void CInstanceBase::NEW_SetOwner(DWORD dwVIDOwner)
{
	m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime()
{
	return m_GraphicThingInstance.GetLocalTime();
}


void CInstanceBase::PushUDPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
}

DWORD	ELTimer_GetServerFrameMSec();

void CInstanceBase::PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID)
{
	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+100;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	kCmdNew.m_uTargetVID = uTargetVID;
	m_kQue_kCmdNew.push_back(kCmdNew);
}

void CInstanceBase::PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{	
	if (IsMainInstance())
	{
		return;
	}

	int nNetworkGap=ELTimer_GetServerFrameMSec()-dwCmdTime;
	
	m_nAverageNetworkGap=(m_nAverageNetworkGap*70+nNetworkGap*30)/100;
	
	/*
	if (m_dwBaseCmdTime == 0)
	{
		m_dwBaseChkTime = ELTimer_GetFrameMSec()-nNetworkGap;
		m_dwBaseCmdTime = dwCmdTime;

		Tracenf("VID[%d] 네트웍갭 [%d]", GetVirtualID(), nNetworkGap);
	}
	*/

	//m_dwBaseChkTime-m_dwBaseCmdTime+ELTimer_GetServerMSec();

	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+m_nAverageNetworkGap;//m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	m_kQue_kCmdNew.push_back(kCmdNew);

	//int nApplyGap=kCmdNew.m_dwChkTime-ELTimer_GetServerFrameMSec();

	//if (nApplyGap<-500 || nApplyGap>500)
	//	Tracenf("VID[%d] NAME[%s] 네트웍갭 [cur:%d ave:%d] 작동시간 (%d)", GetVirtualID(), GetNameString(), nNetworkGap, m_nAverageNetworkGap, nApplyGap);
}

/*
CInstanceBase::TStateQueue::iterator CInstanceBase::FindSameState(TStateQueue& rkQuekStt, DWORD dwCmdTime, UINT eFunc, UINT uArg)
{
	TStateQueue::iterator i=rkQuekStt.begin();
	while (rkQuekStt.end()!=i)
	{
		SState& rkSttEach=*i;
		if (rkSttEach.m_dwCmdTime==dwCmdTime)
			if (rkSttEach.m_eFunc==eFunc)
				if (rkSttEach.m_uArg==uArg)
					break;
		++i;
	}

	return i;
}
*/

BOOL CInstanceBase::__CanProcessNetworkStatePacket()
{
	if (m_GraphicThingInstance.IsDead())
		return FALSE;
	if (m_GraphicThingInstance.IsKnockDown())
		return FALSE;
	if (m_GraphicThingInstance.IsUsingSkill())
		if (!m_GraphicThingInstance.CanCancelSkill())
			return FALSE;

	return TRUE;
}

BOOL CInstanceBase::__IsEnableTCPProcess(UINT eCurFunc)
{
	if (m_GraphicThingInstance.IsActEmotion())
	{
		return FALSE;
	}

	if (!m_bEnableTCPState)
	{
		if (FUNC_EMOTION != eCurFunc)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CInstanceBase::StateProcess()
{	
	while (1)
	{
		if (m_kQue_kCmdNew.empty())
			return;	

		DWORD dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
		DWORD dwCurChkTime = ELTimer_GetServerFrameMSec();	

		if (dwCurChkTime < dwDstChkTime)
			return;

		SCommand kCmdTop = m_kQue_kCmdNew.front();
		m_kQue_kCmdNew.pop_front();	

		TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
		//DWORD dwCmdTime = kCmdTop.m_dwCmdTime;	
		FLOAT fRotDst = kCmdTop.m_fDstRot;
		UINT eFunc = kCmdTop.m_eFunc;
		UINT uArg = kCmdTop.m_uArg;
		UINT uVID = GetVirtualID();	
		UINT uTargetVID = kCmdTop.m_uTargetVID;

		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		/*
		if (IsPC())
			Tracenf("%d cmd: vid=%d[%s] func=%d arg=%d  curPos=(%f, %f) dstPos=(%f, %f) rot=%f (time %d)", 
			ELTimer_GetMSec(),
			uVID, m_stName.c_str(), eFunc, uArg, 
			kPPosCur.x, kPPosCur.y,
			kPPosDst.x, kPPosDst.y, fRotDst, dwCmdTime-m_dwBaseCmdTime);
		*/

		TPixelPosition kPPosDir = kPPosDst - kPPosCur;
		float fDirLen = (float)sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

		//Tracenf("거리 %f", fDirLen);

		if (!__CanProcessNetworkStatePacket())
		{
			Lognf(0, "vid=%d 움직일 수 없는 상태라 스킵 IsDead=%d, IsKnockDown=%d", uVID, m_GraphicThingInstance.IsDead(), m_GraphicThingInstance.IsKnockDown());
			return;
		}

		if (!__IsEnableTCPProcess(eFunc))
		{
			return;
		}

		switch (eFunc)
		{
			case FUNC_WAIT:
			{
				//Tracenf("%s (%f, %f) -> (%f, %f) 남은거리 %f", GetNameString(), kPPosCur.x, kPPosCur.y, kPPosDst.x, kPPosDst.y, fDirLen);
				if (fDirLen > 1.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);

					__EnableSkipCollision();

					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_WAIT;

					if (!IsWalking())
						StartWalking();

					//Tracen("목표정지");
				}
				else
				{
					//Tracen("현재 정지");

					m_isGoing = FALSE;

					if (!IsWaiting())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					SetAdvancingRotation(fRotDst);
					SetRotation(fRotDst);
				}
				break;
			}

			case FUNC_MOVE:
			{
				//NEW_GetSrcPixelPositionRef() = kPPosCur;
				//NEW_GetDstPixelPositionRef() = kPPosDst;
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();
				//m_isSyncMov = TRUE;

				m_kMovAfterFunc.eFunc = FUNC_MOVE;

				if (!IsWalking())
				{
					//Tracen("걷고 있지 않아 걷기 시작");
					StartWalking();
				}
				else
				{
					//Tracen("이미 걷는중 ");
				}
				break;
			}

			case FUNC_COMBO:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot=fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_COMBO;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					//Tracen("대기 공격 정지");

					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					RunComboAttack(fRotDst, uArg);
				}
				break;
			}

			case FUNC_ATTACK:
			{
				if (fDirLen>=50.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();
					//m_isSyncMov = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_ATTACK;

					if (!IsWalking())
						StartWalking();

					//Tracen("너무 멀어서 이동 후 공격");
				}
				else
				{
					//Tracen("노말 공격 정지");

					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					RunNormalAttack(fRotDst);

					//Tracen("가깝기 때문에 워프 공격");
				}
				break;
			}

			case FUNC_MOB_SKILL:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + uArg);
				}
				break;
			}

			case FUNC_EMOTION:
			{
				if (fDirLen>100.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					if (IsMainInstance())
						__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_EMOTION;
					m_kMovAfterFunc.uArg = uArg;
					m_kMovAfterFunc.uArgExpanded = uTargetVID;
					m_kMovAfterFunc.kPosDst = kPPosDst;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					__ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
				}
				break;
			}

			default:
			{
				if (eFunc & FUNC_SKILL)
				{
					if (fDirLen >= 50.0f)
					{
						//NEW_GetSrcPixelPositionRef() = kPPosCur;
						//NEW_GetDstPixelPositionRef() = kPPosDst;
						NEW_SetSrcPixelPosition(kPPosCur);
						NEW_SetDstPixelPosition(kPPosDst);
						m_fDstRot = fRotDst;
						m_isGoing = TRUE;
						//m_isSyncMov = TRUE;
						__EnableSkipCollision();

						m_kMovAfterFunc.eFunc = eFunc;
						m_kMovAfterFunc.uArg = uArg;

						if (!IsWalking())
							StartWalking();

						//Tracen("너무 멀어서 이동 후 공격");
					}
					else
					{
						//Tracen("스킬 정지");

						m_isGoing = FALSE;

						if (IsWalking())
							EndWalking();

						SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
						SetAdvancingRotation(fRotDst);
						SetRotation(fRotDst);

						NEW_UseSkill(0, eFunc & 0x7f, uArg&0x0f, (uArg>>4) ? true : false);
						//Tracen("가깝기 때문에 워프 공격");
					}
				}
				break;
			}
		}
	}
}


void CInstanceBase::MovementProcess()
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	// 렌더링 좌표계이므로 y를 -화해서 더한다.

	TPixelPosition kPPosNext;
	{
		const D3DXVECTOR3 & c_rkV3Mov = m_GraphicThingInstance.GetMovementVectorRef();

		kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
		kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
		kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
	}

	TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSD = NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

	float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x + kPPosDeltaSC.y * kPPosDeltaSC.y);
	float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x + kPPosDeltaSN.y * kPPosDeltaSN.y);
	float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x + kPPosDeltaSD.y * kPPosDeltaSD.y);
	float fRestLen = fTotalLen - fCurLen;

	if (IsMainInstance())
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen<=0.0)
			{
				if (IsWalking())
					EndWalking();

				//Tracen("목표 도달 정지");

				m_isGoing = FALSE;

				BlockMovement();

				if (FUNC_EMOTION == m_kMovAfterFunc.eFunc)
				{
					DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
					DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
					__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
					return;
				}
			}
		}
	}
	else
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			// 만약 렌턴시가 늦어 너무 많이 이동했다면..
			if (fRestLen < -100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);

				float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
				SetAdvancingRotation(fDstRot);
				//Tracenf("VID %d 오버 방향설정 (%f, %f) %f rest %f", GetVirtualID(), kPPosCur.x, kPPosCur.y, fDstRot, fRestLen);			

				// 이동중이라면 다음번에 멈추게 한다
				if (FUNC_MOVE == m_kMovAfterFunc.eFunc)
				{
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
				}
			}
			// 도착했다면...
			else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen)
			{
				if (m_GraphicThingInstance.IsDead() || m_GraphicThingInstance.IsKnockDown())
				{
					__DisableSkipCollision();

					//Tracen("사망 상태라 동작 스킵");

					m_isGoing = FALSE;

					//Tracen("행동 불능 상태라 이후 동작 스킵");
				}
				else
				{
					switch (m_kMovAfterFunc.eFunc)
					{
						case FUNC_ATTACK:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							RunNormalAttack(m_fDstRot);
							break;
						}

						case FUNC_COMBO:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
							break;
						}

						case FUNC_EMOTION:
						{
							m_isGoing = FALSE;
							m_kMovAfterFunc.eFunc = FUNC_WAIT;
							__DisableSkipCollision();
							BlockMovement();

							DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
							DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
							__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
							break;
						}

						case FUNC_MOVE:
						{
							break;
						}

						case FUNC_MOB_SKILL:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + m_kMovAfterFunc.uArg);
							break;
						}

						default:
						{
							if (m_kMovAfterFunc.eFunc & FUNC_SKILL)
							{
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								NEW_UseSkill(0, m_kMovAfterFunc.eFunc & 0x7f, m_kMovAfterFunc.uArg&0x0f, (m_kMovAfterFunc.uArg>>4) ? true : false);
							}
							else
							{
								//Tracenf("VID %d 스킬 공격 (%f, %f) rot %f", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot);

								__DisableSkipCollision();
								m_isGoing = FALSE;

								BlockMovement();
								SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								if (!IsWaiting())
								{
									EndWalking();
								}

								//Tracenf("VID %d 정지 (%f, %f) rot %f IsWalking %d", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot, IsWalking());
							}
							break;
						}
					}

				}
			}

		}
	}

	if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
	{
		float fRotation = m_GraphicThingInstance.GetRotation();
		float fAdvancingRotation = m_GraphicThingInstance.GetAdvancingRotation();
		int iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

		if (DEGREE_DIRECTION_SAME != m_iRotatingDirection)
		{
			if (DEGREE_DIRECTION_LEFT == iDirection)
			{
				fRotation = fmodf(fRotation + m_fRotSpd*m_GraphicThingInstance.GetSecondElapsed(), 360.0f);
			}
			else if (DEGREE_DIRECTION_RIGHT == iDirection)
			{
				fRotation = fmodf(fRotation - m_fRotSpd*m_GraphicThingInstance.GetSecondElapsed() + 360.0f, 360.0f);
			}

			if (m_iRotatingDirection != GetRotatingDirection(fRotation, fAdvancingRotation))
			{
				m_iRotatingDirection = DEGREE_DIRECTION_SAME;
				fRotation = fAdvancingRotation;
			}

			m_GraphicThingInstance.SetRotation(fRotation);
		}

		if (__IsInDustRange() && !IsAffect(AFFECT_EUNHYEONG))
		{ 
			float fDustDistance = NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
			if (IsMountingHorse())
			{
				if (fDustDistance > ms_fHorseDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					AttachEffect(EFFECT_HORSE_DUST);
				}
			}
			else
			{
				if (fDustDistance > ms_fDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					AttachEffect(EFFECT_DUST);
				}
			}
		}
	}
}

void CInstanceBase::__ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition & c_rkPosDst)
{
	if (IsWalking())
		EndWalkingWithoutBlending();

	__EnableChangingTCPState();
	SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
	if (pTargetInstance)
	{
		pTargetInstance->__EnableChangingTCPState();

		if (pTargetInstance->IsWalking())
			pTargetInstance->EndWalkingWithoutBlending();

		WORD wMotionNumber1 = HIWORD(dwMotionNumber);
		WORD wMotionNumber2 = LOWORD(dwMotionNumber);

		int src_job = RaceToJob(GetRace());
		int dst_job = RaceToJob(pTargetInstance->GetRace());

		NEW_LookAtDestInstance(*pTargetInstance);
		m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
		m_GraphicThingInstance.SetRotation(m_GraphicThingInstance.GetTargetRotation());
		m_GraphicThingInstance.SetAdvancingRotation(m_GraphicThingInstance.GetTargetRotation());

		pTargetInstance->NEW_LookAtDestInstance(*this);
		pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber2 + src_job);
		pTargetInstance->m_GraphicThingInstance.SetRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
		pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

		if (pTargetInstance->IsMainInstance())
		{
			IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
			rPlayer.EndEmotionProcess();
		}
	}

	if (IsMainInstance())
	{
		IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.EndEmotionProcess();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int g_iAccumulationTime = 0;

void CInstanceBase::Update()
{
	++ms_dwUpdateCounter;	

	StateProcess();
	m_GraphicThingInstance.PhysicsProcess();
	m_GraphicThingInstance.RotationProcess();
	m_GraphicThingInstance.ComboProcess();
	m_GraphicThingInstance.AccumulationMovement();

	if (m_GraphicThingInstance.IsMovement())
	{
		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		DWORD dwCurTime=ELTimer_GetFrameMSec();
		//if (m_dwNextUpdateHeightTime<dwCurTime)
		{
			m_dwNextUpdateHeightTime=dwCurTime;
			kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
			NEW_SetPixelPosition(kPPosCur);
		}

		// SetMaterialColor
		{
			DWORD dwMtrlColor=__GetShadowMapColor(kPPosCur.x, kPPosCur.y);
			m_GraphicThingInstance.SetMaterialColor(dwMtrlColor);
		}
	}

	m_GraphicThingInstance.UpdateAdvancingPointInstance();

	AttackProcess();
	MovementProcess();

	m_GraphicThingInstance.MotionProcess(IsPC());
	if (IsMountingHorse())
	{
		m_kHorse.actorPointer->HORSE_MotionProcess(FALSE);
	}
	
	while (ProcessDamage());
}

void CInstanceBase::Transform()
{
	if (__IsSyncing())
	{
		//OnSyncing();
	}
	else
	{
		if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
		{
			const D3DXVECTOR3& c_rv3Movment=m_GraphicThingInstance.GetMovementVectorRef();

			float len=(c_rv3Movment.x*c_rv3Movment.x)+(c_rv3Movment.y*c_rv3Movment.y);
			if (len>1.0f)
				OnMoving();
			else
				OnWaiting();	
		}	
	}

	m_GraphicThingInstance.INSTANCEBASE_Transform();
}


void CInstanceBase::Deform()
{
	// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
	if (!__CanRender())
		return;

	++ms_dwDeformCounter;

	m_GraphicThingInstance.INSTANCEBASE_Deform();

	m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
	if (!__CanRender())
		return;

	m_GraphicThingInstance.RenderTrace();
}




void CInstanceBase::Render()
{
	// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
	if (!__CanRender())
		return;

	++ms_dwRenderCounter;

	m_kHorse.Render();
	m_GraphicThingInstance.Render();	
	
	if (CActorInstance::IsDirLine())
	{	
		if (NEW_GetDstPixelPositionRef().x != 0.0f)
		{
			static CScreen s_kScreen;

			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);	
			STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
			
			TPixelPosition px;
			m_GraphicThingInstance.GetPixelPosition(&px);
			D3DXVECTOR3 kD3DVt3Cur(px.x, px.y, px.z);
			//D3DXVECTOR3 kD3DVt3Cur(NEW_GetSrcPixelPositionRef().x, -NEW_GetSrcPixelPositionRef().y, NEW_GetSrcPixelPositionRef().z);
			D3DXVECTOR3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x, -NEW_GetDstPixelPositionRef().y, NEW_GetDstPixelPositionRef().z);

			//printf("%s %f\n", GetNameString(), kD3DVt3Cur.y - kD3DVt3Dest.y);
			//float fdx = NEW_GetDstPixelPositionRef().x - NEW_GetSrcPixelPositionRef().x;
			//float fdy = NEW_GetDstPixelPositionRef().y - NEW_GetSrcPixelPositionRef().y;

			s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
			s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z, kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);
			STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		}
	}	
}

void CInstanceBase::RenderToShadowMap()
{
	if (IsDoor())
		return;

	if (IsBuilding())
		return;

	if (!__CanRender())
		return;

	if (!ExistsMainInstance())
		return;

	CInstanceBase* pkInstMain=GetMainInstancePtr();

	const float SHADOW_APPLY_DISTANCE = 2500.0f;

	float fDistance=NEW_GetDistanceFromDestInstance(*pkInstMain);
	if (fDistance>=SHADOW_APPLY_DISTANCE)
		return;

	m_GraphicThingInstance.RenderToShadowMap();	
}

void CInstanceBase::RenderCollision()
{
	m_GraphicThingInstance.RenderCollisionData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setting & Getting Data

void CInstanceBase::SetVirtualID(DWORD dwVirtualID)
{
	m_GraphicThingInstance.SetVirtualID(dwVirtualID);		
}

void CInstanceBase::SetVirtualNumber(DWORD dwVirtualNumber)
{
	m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int iInstanceType)
{
	m_GraphicThingInstance.SetActorType(iInstanceType);
}

void CInstanceBase::SetAlignment(short sAlignment)
{
	m_sAlignment = sAlignment;
	RefreshTextTailTitle();
}

void CInstanceBase::SetPKMode(BYTE byPKMode)
{
	if (m_byPKMode == byPKMode)
		return;

	m_byPKMode = byPKMode;

	if (IsMainInstance())
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.NotifyChangePKMode();
	}	
}

void CInstanceBase::SetKiller(bool bFlag)
{
	if (m_isKiller == bFlag)
		return;

	m_isKiller = bFlag;
	RefreshTextTail();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
	m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(DWORD dwStateFlags)
{
	if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
		SetKiller(TRUE);
	else
		SetKiller(FALSE);

	if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
		SetPartyMemberFlag(TRUE);
	else
		SetPartyMemberFlag(FALSE);
}

void CInstanceBase::SetComboType(UINT uComboType)
{
	m_GraphicThingInstance.SetComboType(uComboType);
}

const char * CInstanceBase::GetNameString()
{
	return m_stName.c_str();
}

DWORD CInstanceBase::GetLevel()
{
	return m_dwLevel;
}

void CInstanceBase::SetLevel(DWORD dwLevel)
{
	m_dwLevel = dwLevel;
}

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
void CInstanceBase::MobInfoAiFlagRefresh()
{
	std::string strName = CPythonNonPlayer::Instance().GetMonsterName(GetRace());

	if (CPythonSystem::Instance().IsShowMobAIFlag() && CPythonNonPlayer::Instance().IsAggressive(GetRace()))
	{
		strName += "*";
	}

	SetNameString(strName.c_str(), strName.length());

	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}
#endif

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
void CInstanceBase::MobInfoLevelRefresh()
{
	if (CPythonSystem::Instance().IsShowMobLevel())
	{
		m_dwLevel = CPythonNonPlayer::Instance().GetMonsterLevel(GetRace());
	}
	else
	{
		m_dwLevel = 0;
	}

	if (m_dwLevel)
	{
		UpdateTextTailLevel(m_dwLevel);
	}
	else
	{
		CPythonTextTail::Instance().DetachLevel(GetVirtualID());
	}
}
#endif

DWORD CInstanceBase::GetRace()
{
	return m_dwRace;
}

bool CInstanceBase::IsConflictAlignmentInstance(CInstanceBase& rkInstVictim)
{
	if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		return false;

	switch (GetAlignmentType())
	{
		case ALIGNMENT_TYPE_NORMAL:
		case ALIGNMENT_TYPE_WHITE:
			if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
				return true;
			break;
		case ALIGNMENT_TYPE_DARK:
			if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
				return true;
			break;
	}

	return false;
}

void CInstanceBase::SetDuelMode(DWORD type)
{
	m_dwDuelMode = type;
}

DWORD CInstanceBase::GetDuelMode()
{
	return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase& rkInstVictim)
{	
	if (IsPC() && rkInstVictim.IsPC())
	{
		if (PK_MODE_PROTECT == GetPKMode() || PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		{
			return false;
		}
	}

	if (rkInstVictim.IsMining())
	{
		return false;
	}

	if (IsMainInstance())
	{		
		CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
		if(rkPlayer.IsObserverMode())
			return false;
	}

	if (GetVirtualID() == rkInstVictim.GetVirtualID())
		return false;

	if (IsStone())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	else if (IsPC())
	{
		if (rkInstVictim.IsStone())
			return true;

		if (rkInstVictim.IsPC())
		{
			if (GetDuelMode())
			{
				switch(GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return false;
				case DUEL_START:
					if(FindDUELKey(GetVirtualID(),rkInstVictim.GetVirtualID()))
						return true;
					else
						return false;
				}
			}
			if (PK_MODE_GUILD == GetPKMode())
				if (GetGuildID() == rkInstVictim.GetGuildID())
					return false;

			if (rkInstVictim.IsKiller())
				if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
					return true;

			if (PK_MODE_PROTECT != GetPKMode())
			{
				if (PK_MODE_FREE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
				}
				if (PK_MODE_GUILD == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							if (GetGuildID() != rkInstVictim.GetGuildID())
								return true;
				}
			}

			if (IsSameEmpire(rkInstVictim))
			{
				if (IsPVPInstance(rkInstVictim))
					return true;

				if (PK_MODE_REVENGE == GetPKMode())
					if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
						if (IsConflictAlignmentInstance(rkInstVictim))
							return true;
			}
			else
			{
				return true;
			}
		}

		if (rkInstVictim.IsEnemy())
			return true;

		if (rkInstVictim.IsWoodenDoor())
			return true;
	}
	else if (IsEnemy())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsBuilding())
			return true;
		
	}
	else if (IsPoly())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsEnemy())
			return true;
	}
	return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase& rkInstVictim)
{
	return rkInstVictim.CanPickInstance();
}

// 2004. 07. 07. [levites] - 스킬 사용중 타겟이 바뀌는 문제 해결을 위한 코드
bool CInstanceBase::CanChangeTarget()
{
	return m_GraphicThingInstance.CanChangeTarget();
}

// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
bool CInstanceBase::CanPickInstance()
{
	if (!__IsInViewFrustum())
		return false;

	if (IsDoor())
	{
		if (IsDead())
			return false;
	}

	if (IsPC())
	{
		if (IsAffect(AFFECT_EUNHYEONG))
		{
			if (!MainCanSeeHiddenThing())
				return false;
		}
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY))
			return false;
		if (IsAffect(AFFECT_INVISIBILITY))
			return false;
	}

	if (IsDead())
		return false;

	return true;
}

bool CInstanceBase::CanViewTargetHP(CInstanceBase& rkInstVictim)
{
	if (rkInstVictim.IsStone())
		return true;
	if (rkInstVictim.IsWoodenDoor())
		return true;
	if (rkInstVictim.IsEnemy())
		return true;

	return false;
}

BOOL CInstanceBase::IsPoly()
{
	return m_GraphicThingInstance.IsPoly();
}

BOOL CInstanceBase::IsPC()
{
	return m_GraphicThingInstance.IsPC();
}

BOOL CInstanceBase::IsNPC()
{
	return m_GraphicThingInstance.IsNPC();
}

BOOL CInstanceBase::IsEnemy()
{
	return m_GraphicThingInstance.IsEnemy();
}

BOOL CInstanceBase::IsStone()
{
	return m_GraphicThingInstance.IsStone();
}


BOOL CInstanceBase::IsGuildWall()	//IsBuilding 길드건물전체 IsGuildWall은 담장벽만(문은 제외)
{
	return IsWall(m_dwRace);
}


BOOL CInstanceBase::IsResource()
{
	switch (m_dwVirtualNumber)
	{
		case 20047:
		case 20048:
		case 20049:
		case 20050:
		case 20051:
		case 20052:
		case 20053:
		case 20054:
		case 20055:
		case 20056:
		case 20057:
		case 20058:
		case 20059:
		case 30301:
		case 30302:
		case 30303:
		case 30304:
		case 30305:
			return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsWarp()
{
	return m_GraphicThingInstance.IsWarp();
}

BOOL CInstanceBase::IsGoto()
{
	return m_GraphicThingInstance.IsGoto();
}

BOOL CInstanceBase::IsObject()
{
	return m_GraphicThingInstance.IsObject();
}

BOOL CInstanceBase::IsBuilding()
{
	return m_GraphicThingInstance.IsBuilding();
}

BOOL CInstanceBase::IsDoor()
{
	return m_GraphicThingInstance.IsDoor();
}

BOOL CInstanceBase::IsWoodenDoor()
{
	if (m_GraphicThingInstance.IsDoor())
	{
		int vnum = GetVirtualNumber();
		if (vnum == 13000) // 나무문
			return true;
		else if (vnum >= 30111 && vnum <= 30119) // 사귀문
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

BOOL CInstanceBase::IsStoneDoor()
{
	return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

BOOL CInstanceBase::IsFlag()
{
	if (GetRace() == 20035)
		return TRUE;
	if (GetRace() == 20036)
		return TRUE;
	if (GetRace() == 20037)
		return TRUE;

	return FALSE;
}

BOOL CInstanceBase::IsForceVisible()
{
	if (IsAffect(AFFECT_SHOW_ALWAYS))
		return TRUE;

	if (IsObject() || IsBuilding() || IsDoor() )
		return TRUE;

	return FALSE;
}

int	CInstanceBase::GetInstanceType()
{
	return m_GraphicThingInstance.GetActorType();
}

DWORD CInstanceBase::GetVirtualID()
{
	return m_GraphicThingInstance.GetVirtualID();
}

DWORD CInstanceBase::GetVirtualNumber()
{
	return m_dwVirtualNumber;
}

// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
bool CInstanceBase::__IsInViewFrustum()
{
	return m_GraphicThingInstance.isShow();
}

bool CInstanceBase::__CanRender()
{
	if (IsAlwaysRender())
	{
		return true;
	}

	if (!__IsInViewFrustum())
		return false;
	if (IsAffect(AFFECT_INVISIBILITY))
		return false;

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
	float u, v, t;
	return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
	return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase * pkTargetInst)
{
	TPixelPosition TargetPixelPosition;
	pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
	return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition & c_rPixelPosition)
{
	TPixelPosition PixelPosition;
	m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

	float fdx = PixelPosition.x - c_rPixelPosition.x;
	float fdy = PixelPosition.y - c_rPixelPosition.y;

	return sqrtf((fdx*fdx) + (fdy*fdy));
}

float CInstanceBase::GetBaseHeight()
{
	CActorInstance* pkHorse = m_kHorse.GetActorPtr();
	if (!m_kHorse.IsMounting() || !pkHorse)
		return 0.0f;

	DWORD dwHorseVnum = m_kHorse.actorPointer->GetRace();

	float fRaceHeight = CRaceManager::instance().GetRaceHeight(dwHorseVnum);

	if (fRaceHeight == 0.0f)
		return 100.0f;
	else
		return fRaceHeight;
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
	return m_GraphicThingInstance;
}

CActorInstance* CInstanceBase::GetGraphicThingInstancePtr()
{
	return &m_GraphicThingInstance;
}

void CInstanceBase::RefreshActorInstance()
{
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::Refresh(DWORD dwMotIndex, bool isLoop)
{
	RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
	m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode()
{
	m_GraphicThingInstance.SetAddRenderMode();
}

void CInstanceBase::SetModulateRenderMode()
{
	m_GraphicThingInstance.SetModulateRenderMode();
}

void CInstanceBase::SetRenderMode(int iRenderMode)
{
	m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const D3DXCOLOR & c_rColor)
{
	m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::SetBlendRenderingMode()
{
	m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::SetAlphaValue(float alpha)
{
	m_GraphicThingInstance.SetAlphaValue(alpha);
}

float CInstanceBase::GetAlphaValue()
{
	return m_GraphicThingInstance.GetAlphaValue();
}

// Part
void CInstanceBase::SetHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
	{
		return;
	}

	if (IsPC() == false)
	{
		return;
	}

	m_awPart[CRaceData::PART_HAIR] = eHair;
	m_GraphicThingInstance.SetHair(eHair);
}

void CInstanceBase::ChangeHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
	{
		return;
	}

	if (IsPC() == false)
	{
		return;
	}

	if (GetPart(CRaceData::PART_HAIR) == eHair)
	{
		return;
	}

	SetHair(eHair);

	RefreshState(CRaceMotionData::NAME_WAIT, true);
}


void CInstanceBase::SetArmor(DWORD dwArmor)
{
	DWORD dwShape;

	if (__ArmorVnumToShape(dwArmor, &dwShape))
	{
		CItemData* pItemData;

		if (CItemManager::Instance().GetItemDataPointer(dwArmor, &pItemData))
		{
			float fSpecularPower = pItemData->GetSpecularPowerf();
			SetShape(dwShape, fSpecularPower);
			__GetRefinedEffect(pItemData);
			return;
		}

		__ClearArmorRefineEffect();
	}

	SetShape(dwArmor);
}

void CInstanceBase::SetShape(DWORD eShape, float fSpecular)
{
	if (IsPoly())
	{
		m_GraphicThingInstance.SetShape(0);	
	}
	else
	{
		m_GraphicThingInstance.SetShape(eShape, fSpecular);		
	}

	m_eShape = eShape;
}

DWORD CInstanceBase::GetWeaponType()
{
	DWORD dwWeapon = GetPart(CRaceData::PART_WEAPON);
	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwWeapon, &pItemData))
		return CItemData::WEAPON_NONE;

	return pItemData->GetWeaponType();
}

void CInstanceBase::__ClearWeaponRefineEffect()
{
	if (m_swordRefineEffectRight)
	{
		DetachEffect(m_swordRefineEffectRight);
		m_swordRefineEffectRight = 0;
	}
	if (m_swordRefineEffectLeft)
	{
		DetachEffect(m_swordRefineEffectLeft);
		m_swordRefineEffectLeft = 0;
	}
}

void CInstanceBase::__ClearArmorRefineEffect()
{
	if (m_armorRefineEffect)
	{
		DetachEffect(m_armorRefineEffect);
		m_armorRefineEffect = 0;
	}
}

UINT CInstanceBase::__GetRefinedEffect(CItemData* pItem)
{
	DWORD refine = max(pItem->GetRefine() + pItem->GetSocketCount(),CItemData::ITEM_SOCKET_MAX_NUM) - CItemData::ITEM_SOCKET_MAX_NUM;
	switch (pItem->GetType())
	{
	case CItemData::ITEM_TYPE_WEAPON:
		__ClearWeaponRefineEffect();		
		if (refine < 7)	//현재 제련도 7 이상만 이펙트가 있습니다.
			return 0;
		switch(pItem->GetSubType())
		{
		case CItemData::WEAPON_DAGGER:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7+refine-7;
			m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7_LEFT+refine-7;
			break;
		case CItemData::WEAPON_FAN:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_FANBELL_REFINED7+refine-7;
			break;
		case CItemData::WEAPON_ARROW:
		case CItemData::WEAPON_BELL:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7+refine-7;
			break;
		case CItemData::WEAPON_BOW:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_BOW_REFINED7+refine-7;
			break;
		default:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SWORD_REFINED7+refine-7;
		}
		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = AttachEffect(m_swordRefineEffectRight);
		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = AttachEffect(m_swordRefineEffectLeft);
		break;
	case CItemData::ITEM_TYPE_ARMOR:
		__ClearArmorRefineEffect();

		// 갑옷 특화 이펙트
		if (pItem->GetSubType() == CItemData::ARMOR_BODY)
		{
			DWORD vnum = pItem->GetIndex();

			if (12010 <= vnum && vnum <= 12049)
			{
				AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL);
				AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL2);
			}
		}

		if (refine < 7)
			return 0;

		if (pItem->GetSubType() == CItemData::ARMOR_BODY)
		{
			m_armorRefineEffect = EFFECT_REFINED+EFFECT_BODYARMOR_REFINED7+refine-7;
			AttachEffect(m_armorRefineEffect);
		}
		break;
	}
	return 0;
}

bool CInstanceBase::SetWeapon(DWORD eWeapon)
{
	if (IsPoly())
	{
		return false;
	}

	if (__IsShapeAnimalWear())
	{
		return false;
	}

	if (__IsChangableWeapon(eWeapon) == false)
	{
		eWeapon = 0;
	}

	m_GraphicThingInstance.AttachWeapon(eWeapon);
	m_awPart[CRaceData::PART_WEAPON] = eWeapon;

	CItemData* pItemData;
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
	{
		__GetRefinedEffect(pItemData);
	}
	else
	{
		__ClearWeaponRefineEffect();
	}

	return true;
}

void CInstanceBase::ChangeWeapon(DWORD eWeapon)
{
	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		return;
	}

	if (SetWeapon(eWeapon))
	{
		RefreshState(CRaceMotionData::NAME_WAIT, true);
	}
}

bool CInstanceBase::ChangeArmor(DWORD dwArmor)
{
	DWORD eShape;
	__ArmorVnumToShape(dwArmor, &eShape);

	if (GetShape() == eShape)
	{
		return false;
	}

	CAffectFlagContainer kAffectFlagContainer;
	kAffectFlagContainer.CopyInstance(affectFlagContainer);

	DWORD dwVID = GetVirtualID();
	DWORD dwRace = GetRace();
	DWORD eHair = GetPart(CRaceData::PART_HAIR);
	DWORD eWeapon = GetPart(CRaceData::PART_WEAPON);
	float fRot = GetRotation();
	float fAdvRot = GetAdvancingRotation();

	if (IsWalking())
	{
		EndWalking();
	}

	ClearAffects();

	if (!SetRace(dwRace))
	{
		TraceError("CPythonCharacterManager::ChangeArmor - SetRace VID[%d] Race[%d] ERROR", dwVID, dwRace);
		return false;
	}

	SetArmor(dwArmor);
	SetHair(eHair);
	SetWeapon(eWeapon);
	SetRotation(fRot);
	SetAdvancingRotation(fAdvRot);

	__AttachHorseSaddle();

	RefreshState(CRaceMotionData::NAME_WAIT, true);
	SetAffectFlagContainer(kAffectFlagContainer);

	CActorInstance::IEventHandler& rkEventHandler=GetEventHandlerRef();
	rkEventHandler.OnChangeShape();

	return true;
}

bool CInstanceBase::__IsShapeAnimalWear()
{
	if (100 == GetShape() ||
		101 == GetShape() ||
		102 == GetShape() ||
		103 == GetShape())
		return true;

	return false;
}

DWORD CInstanceBase::__GetRaceType()
{
	return m_eRaceType;
}


void CInstanceBase::RefreshState(DWORD dwMotIndex, bool isLoop)
{
	DWORD dwPartItemID = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);

	BYTE byItemType = 0xff;
	BYTE bySubType = 0xff;

	CItemManager & rkItemMgr = CItemManager::Instance();
	CItemData * pItemData;
	
	if (rkItemMgr.GetItemDataPointer(dwPartItemID, &pItemData))
	{
		byItemType = pItemData->GetType();
		bySubType = pItemData->GetWeaponType();
	}

	if (IsPoly())
	{
		SetMotionMode(CRaceMotionData::MODE_GENERAL);
	}
	else if (IsWearingDress())
	{
		SetMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	}
	else if (IsHoldingPickAxe())
	{
		if (m_kHorse.IsMounting())
		{
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
		}
	}
	else if (CItemData::ITEM_TYPE_ROD == byItemType)
	{
		if (m_kHorse.IsMounting())
		{
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_FISHING);
		}
	}
	else if (m_kHorse.IsMounting())
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD); // Only Warrior
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD); // Only Assassin
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_HORSE_FAN); // Only Shaman
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BELL); // Only Shaman
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BOW); // Only Shaman
				break;

			default:
				SetMotionMode(CRaceMotionData::MODE_HORSE);
				break;
		}
	}
	else
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD); // Only Warrior
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD); // Only Assassin
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_BOW); // Only Assassin
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_FAN); // Only Shaman
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_BELL); // Only Shaman
				break;

			case CItemData::WEAPON_ARROW:
			default:
				SetMotionMode(CRaceMotionData::MODE_GENERAL);
				break;
		}
	}

	if (isLoop)
		m_GraphicThingInstance.InterceptLoopMotion(dwMotIndex);
	else
		m_GraphicThingInstance.InterceptOnceMotion(dwMotIndex);

	RefreshActorInstance();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device

void CInstanceBase::RegisterBoundingSphere()
{
	// Stone 일 경우 DeforomNoSkin 을 하면
	// 낙하하는 애니메이션 같은 경우 애니메이션이
	// 바운드 박스에 영향을 미쳐 컬링이 제대로 이루어지지 않는다.
	if (!IsStone())
	{
		m_GraphicThingInstance.DeformNoSkin();
	}

	m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
	return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
	m_GraphicThingInstance.DestroyDeviceObjects();
}

void CInstanceBase::Destroy()
{	
	DetachTextTail();
	
	DismountHorse();

	m_kQue_kCmdNew.clear();
	
	EffectContainerDestroy();
	StoneSmokeDestroy();

	if (IsMainInstance())
		ClearMainInstance();	
	
	m_GraphicThingInstance.Destroy();
	
	__Initialize();
}

void CInstanceBase::__InitializeRotationSpeed()
{
	SetRotationSpeed(c_fDefaultRotationSpeed);
}

void CInstanceBase::__Warrior_Initialize()
{
	m_kWarrior.m_dwGeomgyeongEffect=0;
}

void CInstanceBase::__Initialize()
{
	__Warrior_Initialize();
	StoneSmokeInialize();
	EffectContainerInitialize();
	__InitializeRotationSpeed();

	SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

	affectFlagContainer.Clear();

	m_dwLevel = 0;
	m_dwGuildID = 0;
	m_dwEmpireID = 0;

	m_eType = 0;
	m_eRaceType = 0;
	m_eShape = 0;
	m_dwRace = 0;
	m_bJob = 0;
	m_dwVirtualNumber = 0;

	m_dwBaseCmdTime=0;
	m_dwBaseChkTime=0;
	m_dwSkipTime=0;

	m_GraphicThingInstance.Initialize();

	m_dwAdvActorVID=0;
	m_dwLastDmgActorVID=0;

	m_nAverageNetworkGap=0;
	m_dwNextUpdateHeightTime=0;

	// Moving by keyboard
	m_iRotatingDirection = DEGREE_DIRECTION_SAME;

	// Moving by mouse	
	m_isTextTail = FALSE;
	m_isGoing = FALSE;
	NEW_SetSrcPixelPosition(TPixelPosition(0, 0, 0));
	NEW_SetDstPixelPosition(TPixelPosition(0, 0, 0));

	m_kPPosDust = TPixelPosition(0, 0, 0);


	m_kQue_kCmdNew.clear();

	m_dwLastComboIndex = 0;

	m_swordRefineEffectRight = 0;
	m_swordRefineEffectLeft = 0;
	m_armorRefineEffect = 0;

	m_sAlignment = 0;
	m_byPKMode = 0;
	m_isKiller = false;
	m_isPartyMember = false;

	m_bEnableTCPState = TRUE;

	m_stName = "";

	memset(m_awPart, 0, sizeof(m_awPart));
	memset(m_adwCRCAffectEffect, 0, sizeof(m_adwCRCAffectEffect));
	//memset(m_adwCRCEmoticonEffect, 0, sizeof(m_adwCRCEmoticonEffect));
	memset(&m_kMovAfterFunc, 0, sizeof(m_kMovAfterFunc));

	m_bDamageEffectType = false;
	m_dwDuelMode = DUEL_NONE;
	m_dwEmoticonTime = 0;
	m_IsAlwaysRender = false;
}

CInstanceBase::CInstanceBase()
{
	__Initialize();
}

CInstanceBase::~CInstanceBase()
{
	Destroy();
}

bool CInstanceBase::IsAlwaysRender()
{
	return m_IsAlwaysRender;
}

void CInstanceBase::SetAlwaysRender(bool val)
{
	m_IsAlwaysRender = val;
}

void CInstanceBase::GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

bool CInstanceBase::IsMiningVID(uint32_t vid)
{
	return m_dwMiningVID == vid && GetGraphicThingInstancePtr()->IsMining();
}

void CInstanceBase::StartMining(uint32_t vid)
{
	m_dwMiningVID = vid;
}

void CInstanceBase::CancelMining()
{
	GetGraphicThingInstancePtr()->InterceptLoopMotion(CRaceMotionData::NAME_WAIT);
}
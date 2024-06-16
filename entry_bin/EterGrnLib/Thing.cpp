#include "StdAfx.h"
#include "../eterbase/Debug.h"
#include "Thing.h"
#include "ThingInstance.h"

CGraphicThing::CGraphicThing(const char* c_szFileName) : CResource(c_szFileName)
{
	Initialize();	
}

CGraphicThing::~CGraphicThing()
{
	//OnClear();
	Clear();
}

void CGraphicThing::Initialize()
{
	m_pgrnFile = NULL;
	fileInfo = NULL;
	m_pgrnAni = NULL;

	m_models = NULL;
	m_motions = NULL;
}

void CGraphicThing::OnClear()
{
	if (m_motions)
		delete [] m_motions;

	if (m_models)
		delete [] m_models;

	if (m_pgrnFile)
		GrannyFreeFile(m_pgrnFile);

	Initialize();
}

CGraphicThing::TType CGraphicThing::Type()
{
	static TType s_type = StringToType("CGraphicThing");
	return s_type;
}

bool CGraphicThing::OnIsEmpty() const
{
	return m_pgrnFile ? false : true;
}

bool CGraphicThing::OnIsType(TType type)
{
	if (CGraphicThing::Type() == type)
		return true;

	return CResource::OnIsType(type);
}

bool CGraphicThing::CreateDeviceObjects()
{
	if (!fileInfo)
		return true;
	
	for (int m = 0; m < fileInfo->ModelCount; ++m)
	{
		CGrannyModel & rModel = m_models[m];
		rModel.CreateDeviceObjects();
	}

	return true;
}

void CGraphicThing::DestroyDeviceObjects()
{
	if (!fileInfo)
		return;

	for (int m = 0; m < fileInfo->ModelCount; ++m)
	{
		CGrannyModel & rModel = m_models[m];
		rModel.DestroyDeviceObjects();
	}
}

bool CGraphicThing::CheckModelIndex(int iModel) const
{
	if (!fileInfo)
	{
		Tracef("fileInfo == NULL: %s\n", GetFileName());
		return false;
	}

	assert(fileInfo != NULL);

	if (iModel < 0)
		return false;

	if (iModel >= fileInfo->ModelCount)
		return false;

	return true;
}

bool CGraphicThing::CheckMotionIndex(int iMotion) const
{
	// Temporary
	if (!fileInfo)
		return false;
	// Temporary

	assert(fileInfo != NULL);

	if (iMotion < 0)
		return false;
	
	if (iMotion >= fileInfo->AnimationCount)
		return false;

	return true;
}

CGrannyModel * CGraphicThing::GetModelPointer(int iModel)
{	
	assert(CheckModelIndex(iModel));
	assert(m_models != NULL);
	return m_models + iModel;
}

CGrannyMotion * CGraphicThing::GetMotionPointer(int iMotion)
{
	assert(CheckMotionIndex(iMotion));

	if (iMotion >= fileInfo->AnimationCount)
		return NULL;

	assert(m_motions != NULL);
	return (m_motions + iMotion);
}

int32_t CGraphicThing::GetTextureCount() const
{
	return fileInfo ? max(0, fileInfo->TextureCount) : 0;
}

std::string_view CGraphicThing::GetTexturePath(int32_t textureIndex) const
{
	if (!fileInfo || textureIndex < 0 || textureIndex >= fileInfo->TextureCount) 
	{
		return "";
	}

	return std::string_view(fileInfo->Textures[textureIndex]->FromFileName);
}

int CGraphicThing::GetModelCount() const
{
	if (!fileInfo)
		return 0;

	return (fileInfo->ModelCount);
}

int CGraphicThing::GetMotionCount() const
{
	if (!fileInfo)
		return 0;

	return (fileInfo->AnimationCount);
}

bool CGraphicThing::OnLoad(int iSize, const void * c_pvBuf)
{
	if (!c_pvBuf)
		return false;

	m_pgrnFile = GrannyReadEntireFileFromMemory(iSize, (void *) c_pvBuf);

	if (!m_pgrnFile)
		return false;

	fileInfo = GrannyGetFileInfo(m_pgrnFile);

	if (!fileInfo)
		return false;

	LoadModels();
	LoadMotions();
	return true;
}

// SUPPORT_LOCAL_TEXTURE
static std::string gs_modelLocalPath;

const std::string& GetModelLocalPath()
{
	return gs_modelLocalPath;
}
// END_OF_SUPPORT_LOCAL_TEXTURE

bool CGraphicThing::LoadModels()
{
	assert(m_pgrnFile != NULL);
	assert(m_models == NULL);
	
	if (fileInfo->ModelCount <= 0)
		return false;	

	// SUPPORT_LOCAL_TEXTURE
	const std::string& fileName = GetFileNameString();

	//char localPath[256] = "";
	if (fileName.length() > 2 && fileName[1] != ':')
	{				
		int sepPos = fileName.rfind('\\');
		gs_modelLocalPath.assign(fileName, 0, sepPos+1);
	}
	// END_OF_SUPPORT_LOCAL_TEXTURE

	int modelCount = fileInfo->ModelCount;

	m_models = new CGrannyModel[modelCount];

	for (int m = 0; m < modelCount; ++m)
	{
		CGrannyModel & rModel = m_models[m];
		granny_model * pgrnModel = fileInfo->Models[m];

		if (!rModel.CreateFromGrannyModelPointer(pgrnModel))
			return false;
	}

	GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidVertexSection);
	GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidIndexSection);
	GrannyFreeFileSection(m_pgrnFile, GrannyStandardDeformableIndexSection);
	GrannyFreeFileSection(m_pgrnFile, GrannyStandardTextureSection);
	return true;
}

bool CGraphicThing::LoadMotions()
{
	assert(m_pgrnFile != NULL);
	assert(m_motions == NULL);

	if (fileInfo->AnimationCount <= 0)
		return false;
	
	int motionCount = fileInfo->AnimationCount;

	m_motions = new CGrannyMotion[motionCount];
	
	for (int m = 0; m < motionCount; ++m)
		if (!m_motions[m].BindGrannyAnimation(fileInfo->Animations[m]))
			return false;

	return true;
}

#include "stdafx.h"
#include "start_position.h"
char g_nation_name[4][32] =
{
	"",
	"�ż���",
	"õ����",
	"���뱹",
};

long g_start_map[4] =
{
	0,	// reserved
	1,	// �ż���
	2,	// õ����
	3	// ���뱹
};

DWORD g_start_position[4][2] =
{
	{      0,      0 },	// reserved
	{ 469300, 964200 },	// �ż���
	{  55700, 157900 },	// õ����
	{ 969600, 278400 }	// ���뱹
};


DWORD arena_return_position[4][2] =
{
	{       0,  0       },
	{   347600, 882700  }, // �ھ���
	{   138600, 236600  }, // ������
	{   857200, 251800  }  // �ڶ���
};

uint32_t g_create_position[4][2] = 
{
    {     0,      0},  // None
    {475200, 965700},  // Shinsoo
    { 60010, 155900},  // Chunjo
    {963700, 278500}   // Jinno
};

DWORD g_create_position_canada[4][2] = 
{
	{		0,		0 },
	{ 457100, 946900 },
	{ 45700, 166500 },
	{ 966300, 288300 },	
};
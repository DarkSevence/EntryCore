#include "stdafx.h"
#include "start_position.h"
char g_nation_name[4][32] =
{
	"",
	"신수국",
	"천조국",
	"진노국",
};

long g_start_map[4] =
{
	0,	// reserved
	1,	// 신수국
	2,	// 천조국
	3	// 진노국
};

DWORD g_start_position[4][2] =
{
	{      0,      0 },	// reserved
	{ 469300, 964200 },	// 신수국
	{  55700, 157900 },	// 천조국
	{ 969600, 278400 }	// 진노국
};


DWORD arena_return_position[4][2] =
{
	{       0,  0       },
	{   347600, 882700  }, // 자양현
	{   138600, 236600  }, // 복정현
	{   857200, 251800  }  // 박라현
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
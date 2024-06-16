#pragma once

#include "../../common/cache.h"

class CItemCache : public cache<TPlayerItem>
{
	public:
		CItemCache();
		virtual ~CItemCache();

		void Delete();
		virtual void OnFlush();
};

class CPlayerTableCache : public cache<TPlayerTable>
{
	public:
		CPlayerTableCache();
		virtual ~CPlayerTableCache();

		virtual void OnFlush();

		DWORD GetLastUpdateTime() 
		{ 
			return m_lastUpdateTime; 
		}
};

class CItemPriceListTableCache : public cache<TItemPriceListTable>
{
	public:
		CItemPriceListTableCache(void);
		virtual ~CItemPriceListTableCache() = default;

		void UpdateList(const TItemPriceListTable* pUpdateList);

		virtual void OnFlush(void);

    private:
		static const int s_nMinFlushSec; ///< Minimum cache expire time
};
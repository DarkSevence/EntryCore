#pragma once 

#include <assert.h>

template <typename T> class CSingleton 
{ 
	static T * ms_singleton;
	
	public: 
		CSingleton()
		{ 
			assert(!ms_singleton);
			int offset = (int) (T*) 1 - (int) (CSingleton <T>*) (T*) 1; 
			ms_singleton = (T*) ((int) this + offset);
		} 

		virtual ~CSingleton()
		{ 
			assert(ms_singleton);
			ms_singleton = 0; 
		}

		__forceinline static T & Instance()
		{
			assert(ms_singleton);
			return (*ms_singleton);
		}

		__forceinline static T * InstancePtr()
		{ 
			return (ms_singleton);
		}

		__forceinline static T & instance()
		{
			assert(ms_singleton);
			return (*ms_singleton);
		}
};

template <typename T> T * CSingleton <T>::ms_singleton = 0;

template <typename T> class singleton
{ 
	static T * ms_singleton;
	
	public: 
		singleton()
		{ 
			assert(!ms_singleton);
			int offset = (int) (T*) 1 - (int) (CSingleton <T>*) (T*) 1; 
			ms_singleton = (T*) ((int) this + offset);
		} 

		virtual ~singleton()
		{ 
			assert(ms_singleton);
			ms_singleton = 0; 
		}

		__forceinline static T & Instance()
		{
			assert(ms_singleton);
			return (*ms_singleton);
		}

		__forceinline static T * InstancePtr()
		{ 
			return (ms_singleton);
		}

		__forceinline static T & instance()
		{
			assert(ms_singleton);
			return (*ms_singleton);
		}
};

template <typename T> T * singleton <T>::ms_singleton = 0;
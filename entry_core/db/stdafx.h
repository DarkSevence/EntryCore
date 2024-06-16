#ifndef __INC_METiN_II_DBSERV_STDAFX_H__
#define __INC_METiN_II_DBSERV_STDAFX_H__

#include "../../libthecore/include/stdafx.h"

#ifndef __WIN32__
#include <semaphore.h>
#else
#define isdigit iswdigit
#define isspace iswspace
#endif

#include <map>
#include <set>
#include <list>
#include <memory>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include "../../common/length.h"
#include "../../common/tables.h"
#include "../../common/singleton.h"
#include "../../common/utils.h"
#include "../../common/service.h"
#include "../../common/global_config.h"


#endif

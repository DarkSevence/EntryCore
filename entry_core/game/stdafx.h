// Basic features
// Enable or disable memory pooling for specific object types
//#define M2_USE_POOL
// Enable or disable heap allocation debugging
//#define DEBUG_ALLOC

#include "debug_allocator.h"

#include "../../libthecore/include/stdafx.h"

#include "../../common/singleton.h"
#include "../../common/utils.h"
#include "../../common/service.h"
#include "../../common/global_config.h"

#include "../../common/const.hpp"
#include <algorithm>
#include <cmath>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <vector>
#include <cfloat>
#include <array>
#include <numbers>
#include <format>
#include <optional>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "typedef.h"
#include "locale.hpp"
#include "event.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../../libfmt/include/fmt/format.h"
#include <boost/tokenizer.hpp>

#define PASSES_PER_SEC(sec) ((sec) * passes_per_sec)

#ifndef M_PI
#define M_PI    3.14159265358979323846 /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923 /* pi/2 */
#endif

#define IN
#define OUT

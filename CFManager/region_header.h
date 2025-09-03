#pragma once

#if defined(REGION_CHINA)
#include "cn_classes.h"
#include "cn_offsets.h"
static constexpr char const* kRegionVersion = "china";
#elif defined(REGION_NA)
#include "classes.h"
#include "offsets.h"
static constexpr char const* kRegionVersion = "NA";
#endif
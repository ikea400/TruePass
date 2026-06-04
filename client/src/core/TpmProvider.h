#pragma once

#ifdef _WIN32
#include "WinTpmProvider.h"
using TpmProvider = WinTpmProvider;
#elif defined(__linux__)
#include "LinuxTpmProvider.h"
using TpmProvider = LinuxTpmProvider;
#endif
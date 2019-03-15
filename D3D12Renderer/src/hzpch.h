#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <vector>
#include <map>

#include <inttypes.h>

#ifdef HZ_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <wrl.h>
	#include <dxgi1_4.h>
	#include <d3dcompiler.h>
	#include <d3d12.h>
	#include "d3dx12.h"
#endif
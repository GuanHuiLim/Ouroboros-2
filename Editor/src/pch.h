/************************************************************************************//*!
\file           pch.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2021
\brief          Pre-Compiled Header that will be used throughout the entire project.

                Pre-compiled headers should include files that are frequently used,
                usually external libraries that wont be changed. It helps a lot with
                saving compilation time.

                Explaination of PCH: https://www.youtube.com/watch?v=eSI4wctZUto&t=955s

                Premake :
                pchheader ("filename")
                pchsource ("directory to pch's cpp file")

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

//#ifdef OO_PLATFORM_WINDOWS
//#include <Windows.h>
//#endif

// commonly used internal project files
//#include <oom/oom.hpp>
//#include <rttr/registration>

// Utility files
//#include "Utility/Bitmask.h"
//#include "Utility/EventCallback.h"

// Commonly use external libraries
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <type_traits>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <variant>

#include <functional>
#include <algorithm>
#include <memory>
#include <cstdint>

// multithreading and time
#include <thread>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

// Data structures
#include <string>
#include <vector>
#include <array>
#include <initializer_list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <deque>
#include <stack>
#include <queue>

#include <stdexcept>

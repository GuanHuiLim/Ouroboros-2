/************************************************************************************//*!
\file           pch.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2022
\brief          Pre-Compiled Header that will be used throughout the entire project.

                Pre-compiled headers should include files that are frequently used,
                usually external libraries that wont be changed. It helps a lot with
                saving compilation time.

                Explaination of PCH: https://www.youtube.com/watch?v=eSI4wctZUto&t=955s

                Premake :
                pchheader ("filename")
                pchsource ("directory to pch's cpp file")

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

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
#include <execution>
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

// glm
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// audio
#include <fmod.hpp>

// Utility files
//#include "Utility/Bitmask.h"
//#include "Utility/EventCallback.h"
#include "Ouroboros/Core/Base.h"
#include "Utility/Random.h"

//Tracy
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
//Optick
#include <optick.h>

// ecs
//#include <Archetypes_Ecs/src/A_Ecs.h>
#include "Ouroboros/ECS/ECS.h"
#include "Ouroboros/ECS/DeferredComponent.h"
#include "Ouroboros/ECS/DuplicatedComponent.h"

// threadpool
#include <JobSystem/src/final/jobs.h>

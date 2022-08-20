//
//          Tracy profiler
//         ----------------
//
// For fast integration, compile and
// link with this source file (and none
// other) in your executable (or in the
// main DLL / shared object on multi-DLL
// projects).
//

// Define TRACY_ENABLE to enable profiler.
#include "pch.h"

#include <tracy/common/TracySystem.cpp>

#ifdef TRACY_ENABLE

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif

#include <tracy/common/tracy_lz4.cpp>
#include <tracy/client/TracyProfiler.cpp>
#include <tracy/client/TracyCallstack.cpp>
#include <tracy/client/TracySysTime.cpp>
#include <tracy/client/TracySysTrace.cpp>
#include <tracy/common/TracySocket.cpp>
#include <tracy/client/tracy_rpmalloc.cpp>
#include <tracy/client/TracyDxt1.cpp>

#if TRACY_HAS_CALLSTACK == 2 || TRACY_HAS_CALLSTACK == 3 || TRACY_HAS_CALLSTACK == 4 || TRACY_HAS_CALLSTACK == 6
#  include <tracy/libbacktrace/alloc.cpp>
#  include <tracy/libbacktrace/dwarf.cpp>
#  include <tracy/libbacktrace/fileline.cpp>
#  include <tracy/libbacktrace/mmapio.cpp>
#  include <tracy/libbacktrace/posix.cpp>
#  include <tracy/libbacktrace/sort.cpp>
#  include <tracy/libbacktrace/state.cpp>
#  if TRACY_HAS_CALLSTACK == 4
#    include <libbacktrace/macho.cpp>
#  else
#    include <libbacktrace/elf.cpp>
#  endif
#endif

#ifdef _MSC_VER
#  pragma comment(lib, "ws2_32.lib")
#  pragma comment(lib, "dbghelp.lib")
#  pragma comment(lib, "advapi32.lib")
#  pragma comment(lib, "user32.lib")
#  pragma warning(pop)
#endif

#endif

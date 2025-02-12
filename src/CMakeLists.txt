project(non-smooth-lyapunov-analysis C CXX)
cmake_minimum_required(VERSION 3.13)
# Check that the prerequisite libraries are installed and visible
message(STATUS "Starting dependency check...")

link_directories(/usr/local/lib/ibex/3rd /usr/local/lib/ibex)
include_directories(/usr/local/include/ibex /usr/local/include/ibex/3rd)

set (INCLUDE_DEPS ibex gdtoa ultim soplex gaol)
set (LIB_DEPS ibex gdtoa ultim soplex gaol)

set (ibex_lib ibex)
set (gdtoa_lib gdtoa)
set (ultim_lib ultim)
set (soplex_lib soplex)
set (gaol_lib gaol)

set (ibex_header ibex.h)
set (gaol_header ibex/3rd/gaol/gaol.h)
set (gdtoa_header ibex/3rd/gdtoa/gdtoa.h)
set (ultim_header ibex/3rd/MathLib.h)
set (soplex_header ibex/3rd/soplex.h)

#target_link_libraries(gdtoa /usr/local/lib/ibex/3rd/libgdtoa.a)

foreach (LIB ${LIB_DEPS})
    string (TOUPPER ${LIB} LIB_UPPER)
    find_library(${LIB_UPPER}_LIBRARY NAMES ${${LIB}_lib} PATHS /usr/local/lib/ibex/3rd)
    if (NOT ${LIB_UPPER}_LIBRARY)
        message(FATAL_ERROR "\t ${LIB} library not found.")
    endif()
    add_library(${LIB} UNKNOWN IMPORTED)
    set_property(TARGET ${LIB} PROPERTY IMPORTED_LOCATION ${${LIB_UPPER}_LIBRARY})
    message("\t ${LIB} found in ${${LIB_UPPER}_LIBRARY}")

endforeach ()

foreach (LIB ${INCLUDE_DEPS})
    string(TOUPPER ${LIB} HEADER_PKG)
    set (HEADER ${${LIB}_header})
    find_path(${HEADER_PKG}_INCLUDE_DIR NAMES ${HEADER})
    if (NOT ${HEADER_PKG}_INCLUDE_DIR)
        message(FATAL_ERROR "\t ${HEADER} header not found.")
    endif()
    message("\t ${HEADER} found in ${${HEADER_PKG}_INCLUDE_DIR}")
    set (DEP_INCLUDE_DIRS ${DEP_INCLUDE_DIRS} ${${HEADER_PKG}_INCLUDE_DIR})
endforeach ()

message (STATUS "Finished dependency check.")


# this is not yet revised
set( CMAKE_CXX_FLAGS "-O0 " )
add_definitions("-c")
include_directories(${DEP_INCLUDE_DIRS})

set(SOURCE_FILES
    boxManager.h boxManager.cpp
    ibex_P_StructNSLyap.h ibex_P_StructNSLyap.cpp
    utilities.h utilities.cpp
    debug.h debug.cpp
    bnb_solver.cpp)

add_executable(bnb_solver ${SOURCE_FILES})

target_link_libraries(bnb_solver
        ibex gaol gdtoa ultim soplex
)

set(F_SOURCE_FILES
    ibex_P_StructNSLyap.h ibex_P_StructNSLyap.cpp
    debug.h debug.cpp
    formal_solver.cpp)

add_executable(formal_solver ${F_SOURCE_FILES})

target_link_libraries(formal_solver
        ibex gaol gdtoa ultim soplex
)

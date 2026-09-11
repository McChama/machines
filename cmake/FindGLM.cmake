# - Try to find GLM
# Once done this will define
#
#  GLM_FOUND - system has GLM
#  GLM_INCLUDE_DIR - the GLM include directory
#
# GLM is header-only, so no library to find.

FIND_PATH(GLM_INCLUDE_DIR glm/glm.hpp
    PATHS
    ${WINDOWS_DEPENDENCIES_DIR}/include
    /usr/include
    /usr/local/include
    /opt/local/include
    PATH_SUFFIXES include
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLM DEFAULT_MSG GLM_INCLUDE_DIR)

MARK_AS_ADVANCED(GLM_INCLUDE_DIR)

# - Find Rapidxml++
# Find the native Rapidxml++ includes and library
#
#   RAPIDXML_FOUND       - True if JsonCpp found.
#   RAPIDXML_INCLUDE_DIR - where to find tinyxml.h, etc.
#   RAPIDXML_LIBRARIES   - List of libraries when using JsonCpp.
#

# Look for the header file.
find_path(RAPIDXML_INCLUDE_DIR NAMES rapidxml/rapidxml.hpp)
mark_as_advanced(RAPIDXML_INCLUDE_DIR)

if(RAPIDXML_FOUND)
    set(RAPIDXML_INCLUDE_DIRS $ {RAPIDXML_INCLUDE_DIR})
endif()


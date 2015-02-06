# - Find JsonCpp
# Find the native JsonCpp includes and library
#
#   JSONCPP_FOUND       - True if JsonCpp found.
#   JSONCPP_INCLUDE_DIR - where to find tinyxml.h, etc.
#   JSONCPP_LIBRARIES   - List of libraries when using JsonCpp.
#

# Look for the header file.
find_path(JSONCPP_INCLUDE_DIR NAMES jsoncpp/json/json.h)
mark_as_advanced(JSONCPP_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
find_library(JSONCPP_LIBRARY NAMES
             jsoncpp
             libjson_linux-gcc-4.7_libmt
             libjson_linux-gcc-4.6_libmt
            )
mark_as_advanced(JSONCPP_LIBRARY)

if(JSONCPP_FOUND)
    set(JSONCPP_LIBRARIES $ {JSONCPP_LIBRARY})
    set(JSONCPP_INCLUDE_DIRS $ {JSONCPP_INCLUDE_DIR})
endif()


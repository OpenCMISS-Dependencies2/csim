# - Find CVODE
# Find the CVODE includes and library
# This module defines
#  CVODE_INCLUDE_DIR, where to find CVODE header files.
#  CVODE_LIBRARIES, the libraries needed to use CVODE.
#  CVODE_FOUND, If false, do not try to use CVODE.
# also defined, but not for general use are
#  CVODE_LIBRARY, where to find CVODE.
#  CVODE_NVECTOR_SERIAL_LIBRARY, where to find NVECTOR_SERIAL.

FIND_PATH(CVODE_INCLUDE_DIR cvode/cvode.h
    ${DEPENDENCY_DIR}/include
    /usr/include/
    /usr/local/include/
)

FIND_LIBRARY(CVODE_LIBRARY sundials_cvode
    ${DEPENDENCY_DIR}/lib
    /usr/lib
    /usr/local/lib
)

FIND_LIBRARY(CVODE_NVECTOR_SERIAL_LIBRARY sundials_nvecserial
    ${CSIM_DEPENDENCY_DIR}/lib
    /usr/lib
    /usr/local/lib
)

IF (CVODE_INCLUDE_DIR AND CVODE_LIBRARY AND CVODE_NVECTOR_SERIAL_LIBRARY)
   SET(CVODE_LIBRARIES ${CVODE_LIBRARY} ${CVODE_NVECTOR_SERIAL_LIBRARY})
   SET(CVODE_FOUND TRUE)
ENDIF (CVODE_INCLUDE_DIR AND CVODE_LIBRARY AND CVODE_NVECTOR_SERIAL_LIBRARY)


IF (CVODE_FOUND)
   IF (NOT CVODE_FIND_QUIETLY)
      MESSAGE(STATUS "Found CVODE: ${CVODE_LIBRARIES}")
   ENDIF (NOT CVODE_FIND_QUIETLY)
ELSE (CVODE_FOUND)
   IF (CVODE_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CVODE")
   ELSE (CVODE_FIND_REQUIRED)
      MESSAGE(STATUS "CVODE not found")
   ENDIF (CVODE_FIND_REQUIRED)
ENDIF (CVODE_FOUND)

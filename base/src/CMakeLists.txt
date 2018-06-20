file(GLOB_RECURSE HEADERFILES "../include/*")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
ADD_LIBRARY(csbase ${DIR_SRCS} ${HEADERFILES})
INCLUDE_DIRECTORIES(
    ../include
    ${TOPDIR}/bases/cg/include
    ${TOPDIR}/miniboost
)


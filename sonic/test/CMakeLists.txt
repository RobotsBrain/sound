set(EXECUTABLE_OUTPUT_PATH ${OUTDIR}/test)

include_directories(
	${TOPDIR}/base/include/
    ${TOPDIR}/sonic/include/
    ${TOPDIR}/sonic/src/
)

link_directories(
)


#################################
add_executable(test_fft test_fft.cpp)
target_link_libraries(test_fft sonic)


#################################
add_executable(test_genwave test_genwave.cpp)
target_link_libraries(test_genwave sonic)


#################################
add_executable(test_listen test_listen.cpp)
target_link_libraries(test_listen sonic csbase pthread)
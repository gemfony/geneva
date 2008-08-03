SET (gbitprobabilitytest_SRCS 
	gbitprobabilitytest
)

SET ( GENEVAINCLUDEDIRS
	../../include
)

include_directories ( 
	${GENEVAINCLUDEDIRS} 
	${Boost_INCLUDE_DIRS}
)

link_directories (
	${GENEVA_BINARY_DIR}/src
	${Boost_LIBRARY_DIRS}	
)

ADD_EXECUTABLE(gbitprobabilitytest
	${gbitprobabilitytest_SRCS}
)

target_link_libraries (gbitprobabilitytest
	geneva
	${Boost_LIBRARIES}
)

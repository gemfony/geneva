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
	boost_thread-gcc43-mt
	boost_system-gcc43-mt
	boost_serialization-gcc43-mt
	boost_date_time-gcc43-mt
)

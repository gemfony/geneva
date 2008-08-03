SET (gboundeddoubletest_SRCS 
	gboundeddoubletest
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

ADD_EXECUTABLE(gboundeddoubletest
	${gboundeddoubletest_SRCS}
)

target_link_libraries (gboundeddoubletest
	geneva
	${Boost_LIBRARIES}
)

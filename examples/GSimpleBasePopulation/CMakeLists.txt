SET (gsimplebasepopulation_SRCS 
	gsimplebasepopulation
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

ADD_EXECUTABLE(gsimplebasepopulation
	${gsimplebasepopulation_SRCS}
)

target_link_libraries (gsimplebasepopulation
	geneva
	${Boost_LIBRARIES}
)

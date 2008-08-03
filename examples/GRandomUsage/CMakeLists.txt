SET (grandomusage_SRCS 
	grandomusage
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

ADD_EXECUTABLE(grandomusage
	${grandomusage_SRCS}
)

target_link_libraries (grandomusage
	geneva
    ${Boost_LIBRARIES}
)

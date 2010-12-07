#
# This file is based on the PCH_GCC4_v2.cmake macro posted in
#   http://www.cmake.org/Bug/view.php?id=1260
# and heavily customized to support out-of-tree builds and a more flexible use.
#

#
# ADD_PCH_RULE( _header_filename _src_list )
#
# _header_filename
#     header to make a .gch
#
# _src_list 
#     the variable name (do not use ${..}) which contains a list of sources (a.cpp b.cpp ...)
#     This macro will append a header file to it, then this src_list can be used in
#     "ADD_EXECUTABLE(...)"
#
#
# Use this macro before "ADD_EXECUTABLE()". For each header added with this macro
# a .gch file will then be generated and gcc will use it.
#     (Add -Winvalid-pch to the cpp flags to verify)
# Make clean should delete the pch file.
#
# Note that the use of pre-compiled headers requires huge amounts of disk space!
# Even simple include files generate compiled header files of ~100MB.
# Define BUILDPCH=0 if you want to avoid activating PCH use even if supported.
#
# Example: ADD_PCH_RULE( headers.h myprog_SRCS )
#

# Make sure the compiler is GCC and supports PCH
IF ( CMAKE_COMPILER_IS_GNUCXX )

	EXEC_PROGRAM(
		${CMAKE_CXX_COMPILER}  
		ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion 
		OUTPUT_VARIABLE gcc_compiler_version
	)
	IF ( gcc_compiler_version MATCHES "4\\.[0-9]\\.[0-9]" )
		SET( PCHSupport_FOUND TRUE )
	ELSE ( gcc_compiler_version MATCHES "4\\.[0-9]\\.[0-9]" )
		IF ( gcc_compiler_version MATCHES "3\\.4\\.[0-9]" )
			SET( PCHSupport_FOUND TRUE )
		ENDIF ( gcc_compiler_version MATCHES "3\\.4\\.[0-9]" )
	ENDIF ( gcc_compiler_version MATCHES "4\\.[0-9]\\.[0-9]" )

ENDIF ( CMAKE_COMPILER_IS_GNUCXX )

# Activate PCH support by default
IF ( NOT DEFINED BUILDPCH )
	SET( BUILDPCH FALSE )
ENDIF ( NOT DEFINED BUILDPCH )

# Inform the user
SET( PCHSupport_USE FALSE )
IF ( BUILDPCH )
	IF ( PCHSupport_FOUND )
		MESSAGE( "Using Pre-Compiled Headers" )
		SET( PCHSupport_USE TRUE )
	ELSE ( PCHSupport_FOUND )
		MESSAGE( "Pre-Compiled Headers will not be used, unsupported compiler" )
	ENDIF ( PCHSupport_FOUND )
ELSE ( BUILDPCH )
	MESSAGE( "Pre-Compiled Headers will not be used, BUILDPCH is set to false" )
ENDIF ( BUILDPCH )


# The actual macro
MACRO( ADD_PCH_RULE _header_filename _src_list )

	IF ( PCHSupport_USE )

		#MESSAGE( "Info: Adding Pre-Compiled Header for ${_header_filename}" )

		IF ( NOT CMAKE_BUILD_TYPE )
			MESSAGE( FATAL_ERROR "This is the ADD_PCH_RULE macro: you must set CMAKE_BUILD_TYPE!" )
		ENDIF ( NOT CMAKE_BUILD_TYPE )

		SET( _gch_filename "${CMAKE_CURRENT_BINARY_DIR}/${_header_filename}_${CMAKE_BUILD_TYPE}.gch" )
		LIST( APPEND ${_src_list} ${_gch_filename} )

		# Add $CMAKE_CURRENT_BINARY_DIR to the includes, as it is where the precompiled headers are
		INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

		SET( _args ${CMAKE_CXX_FLAGS} )
		LIST( APPEND _args -c ${CMAKE_CURRENT_SOURCE_DIR}/${_header_filename} -o ${_gch_filename} )
		GET_DIRECTORY_PROPERTY( _include_dirs INCLUDE_DIRECTORIES )
		FOREACH ( _inc ${_include_dirs} )
			LIST( APPEND _args "-I" ${_inc} )
		ENDFOREACH ( _inc ${DIRINC} )
		SEPARATE_ARGUMENTS( _args )
		ADD_CUSTOM_COMMAND( OUTPUT ${_gch_filename}
			COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_args}
			DEPENDS ${_header_filename}
		)

	ELSE ( PCHSupport_USE )

		# Just do nothing, a normal configuration takes place
		#MESSAGE( "Info: Not adding Pre-Compiled Header for ${_header_filename}, unsupported" )

	ENDIF ( PCHSupport_USE )

ENDMACRO( ADD_PCH_RULE )

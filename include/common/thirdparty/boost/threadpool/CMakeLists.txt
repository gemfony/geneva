CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_SUBDIRECTORY (detail)

SET ( THIRDPARTYBOOSTTHREADPOOLINCLUDES
	future.hpp
	pool_adaptors.hpp
	pool.hpp
	scheduling_policies.hpp
	shutdown_policies.hpp
	size_policies.hpp
	task_adaptors.hpp
)

INSTALL ( FILES ${THIRDPARTYBOOSTTHREADPOOLINCLUDES} DESTINATION include/common/thirdparty/boost/threadpool
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( GENEVASCRIPTS
	genevaConfig.gcfg
	prepareBuild.sh
	rsyncList.sh
	sampleHostFile.hcfg
	sshList.sh
	startLocalJobs.sh
)

INSTALL ( FILES ${GENEVASCRIPTS} DESTINATION scripts
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)

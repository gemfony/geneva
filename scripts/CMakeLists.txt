CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( GENEVACONFFILES
	README
	genevaConfig.gcfg
	sampleHostFile.hcfg
)

SET ( GENEVASCRIPTS
	prepareBuild.sh
	rsyncList.sh
	sshList.sh
	startLocalJobs.sh
)

INSTALL ( FILES ${GENEVACONFFILES} DESTINATION scripts )

INSTALL ( PROGRAMS ${GENEVASCRIPTS} DESTINATION scripts )

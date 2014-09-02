########################################################################
# Pointwise - Proprietary software product of Pointwise, Inc.
#             Copyright (c) 1995-2012 Pointwise, Inc.
#             All rights reserved.
#
# modulelocal.mk for src\plugins\CaeUnsDualMesh plugin
########################################################################

#-----------------------------------------------------------------------
# OPTIONAL, locally defined plugin make targets. If a plugin developer wants
# to extend a plugin's make scheme, they should create a modulelocal.mk file
# in the plugin's base folder. To provide for future SDK upgrades, the standard
# module.mk file should NOT be directly edited.
#
# Uncomment, copy and/or edit the sections below as needed.
#
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Adds plugin specific source files to the build.
# These files will be compiled and then linked to the plugin.
# The file paths are relative to the project path macro CaeUnsDualMesh_LOC.
# For example,
#    myFile.cxx is located in $(CaeUnsDualMesh_LOC)/myFile.cxx
#    sub/myOtherFile.cxx is located in $(CaeUnsDualMesh_LOC)/sub/myOtherFile.cxx
#
CaeUnsDualMesh_CXXFILES_PRIVATE := \
    FanSorter.cxx \
    $(NULL)

#-----------------------------------------------------------------------
# Adds plugin specific include flags to the build.
#
#CaeUnsDualMesh_INCL_PRIVATE := \
#	-I./path/to/a/private/include/folder \
#	-I./path/to/another/private/include/folder \
#	$(NULL)
CaeUnsDualMesh_INCL_PRIVATE := \
	-I../cml \
	-I.. \
	$(NULL)

#-----------------------------------------------------------------------
# Adds plugin specific compile flags to the build.
#
CaeUnsDualMesh_CXXFLAGS_PRIVATE := \
    -std=c++0x \
    $(NULL)


#-----------------------------------------------------------------------
# Adds plugin specific -lfile and -Llibpath to build.
# These flags will be added to the link.
#
#CaeUnsDualMesh_LIBS_PRIVATE := \
#	-L./path/to/a/private/lib/folder \
#	-lprivate \
#	-L./path/to/another/private/lib/folder \
#	-lprivate2 \
#	$(NULL)

#-----------------------------------------------------------------------
# Adds plugin specific link flags to the build.
#
#CaeUnsDualMesh_LDFLAGS_PRIVATE := \
#	$(NULL)

#-----------------------------------------------------------------------
# Add any locally defined targets that do NOT produce an output object.
# This would include targets used for cleaning, printing, etc. These targets
# will be automatically added the .PHONY target.
#
#CaeUnsDualMesh_MAINT_TARGETS_PRIVATE = \
#	$(NULL)

#-----------------------------------------------------------------------
# Sample macro. Prefix with CAE name to prevent conflicts.
#
#CaeUnsDualMesh_DUMMY = \
#	DUMMY1 \
#	DUMMY2 \
#	$(NULL)

#-----------------------------------------------------------------------
# Sample target. Prefix with CAE name to prevent conflicts.
#
#CaeUnsDualMesh_sample: CaeUnsDualMesh_clean CaeUnsDualMesh
#	@echo "done building CaeUnsDualMesh"

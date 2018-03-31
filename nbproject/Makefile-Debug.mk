#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/TranMgr.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-lPocoFoundation -lPocoUtil -lPocoNet -lPocoJSON -lPocoCrypto -lpqxx -lpq
CXXFLAGS=-lPocoFoundation -lPocoUtil -lPocoNet -lPocoJSON -lPocoCrypto -lpqxx -lpq

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/local/lib/ -L/usr/pgsql-9.4/lib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /home/abhishek/TranMgr

/home/abhishek/TranMgr: ${OBJECTFILES}
	${MKDIR} -p /home/abhishek
	${LINK.cc} -o /home/abhishek/TranMgr ${OBJECTFILES} ${LDLIBSOPTIONS} -lpqxx -lpq

${OBJECTDIR}/TranMgr.o: TranMgr.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/ -I/usr/pgsql-9.4/include -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TranMgr.o TranMgr.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

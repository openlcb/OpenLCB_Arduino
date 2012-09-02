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
CND_PLATFORM=GNU-MacOSX
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile.nb

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/OlcbStream.o \
	${OBJECTDIR}/Configuration.o \
	${OBJECTDIR}/Datagram.o \
	${OBJECTDIR}/PCE.o \
	${OBJECTDIR}/Event.o \
	${OBJECTDIR}/LinkControl.o \
	${OBJECTDIR}/OpenLcbCanBuffer.o \
	${OBJECTDIR}/NodeMemory.o \
	${OBJECTDIR}/ButtonProtocol.o \
	${OBJECTDIR}/PIP.o \
	${OBJECTDIR}/SNII.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbcommoncan.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbcommoncan.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbcommoncan.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbcommoncan.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbcommoncan.a

${OBJECTDIR}/OlcbStream.o: OlcbStream.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/OlcbStream.o OlcbStream.cpp

${OBJECTDIR}/Configuration.o: Configuration.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/Configuration.o Configuration.cpp

${OBJECTDIR}/Datagram.o: Datagram.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/Datagram.o Datagram.cpp

${OBJECTDIR}/PCE.o: PCE.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/PCE.o PCE.cpp

${OBJECTDIR}/Event.o: Event.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/Event.o Event.cpp

${OBJECTDIR}/LinkControl.o: LinkControl.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/LinkControl.o LinkControl.cpp

${OBJECTDIR}/OpenLcbCanBuffer.o: OpenLcbCanBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/OpenLcbCanBuffer.o OpenLcbCanBuffer.cpp

${OBJECTDIR}/NodeMemory.o: NodeMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/NodeMemory.o NodeMemory.cpp

${OBJECTDIR}/ButtonProtocol.o: ButtonProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/ButtonProtocol.o ButtonProtocol.cpp

${OBJECTDIR}/PIP.o: PIP.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/PIP.o PIP.cpp

${OBJECTDIR}/SNII.o: SNII.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../OlcbDesktopCAN -MMD -MP -MF $@.d -o ${OBJECTDIR}/SNII.o SNII.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbcommoncan.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

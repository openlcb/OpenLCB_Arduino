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
	${OBJECTDIR}/EEPROM.o \
	${OBJECTDIR}/BG.o \
	${OBJECTDIR}/OpenLcbCanInterface_local.o \
	${OBJECTDIR}/OpenLcbCanInterface.o \
	${OBJECTDIR}/Arduino.o \
	${OBJECTDIR}/ButtonLed.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbdesktopcan.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbdesktopcan.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbdesktopcan.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbdesktopcan.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbdesktopcan.a

${OBJECTDIR}/EEPROM.o: EEPROM.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../libraries/OlcbCommonCAN -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/EEPROM.o EEPROM.cpp

${OBJECTDIR}/BG.o: BG.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../libraries/OlcbCommonCAN -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/BG.o BG.cpp

${OBJECTDIR}/OpenLcbCanInterface_local.o: OpenLcbCanInterface_local.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../libraries/OlcbCommonCAN -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/OpenLcbCanInterface_local.o OpenLcbCanInterface_local.cpp

${OBJECTDIR}/OpenLcbCanInterface.o: OpenLcbCanInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../libraries/OlcbCommonCAN -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/OpenLcbCanInterface.o OpenLcbCanInterface.cpp

${OBJECTDIR}/Arduino.o: Arduino.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../libraries/OlcbCommonCAN -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/Arduino.o Arduino.cpp

${OBJECTDIR}/ButtonLed.o: ButtonLed.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../libraries/OlcbCommonCAN -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/ButtonLed.o ButtonLed.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libolcbdesktopcan.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

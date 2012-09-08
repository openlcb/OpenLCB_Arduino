//==============================================================
// OlcbIo
//   For the RailStars Io board, using the Olcb* libraries
// 
//   setup() determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2010, 2012
//      based on OlcbBasicNode
//==============================================================

// next line for stand-alone compile
#include <Arduino.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "logging.h"

// The following lines are needed because the Arduino environment 
// won't search a library directory unless the library is included 
// from the top level file (this file)
#include <EEPROM.h>
#include <can.h>

#include "OlcbCommonVersion.h"
#include "NodeID.h"

// init for serial communications if used
#define         BAUD_RATE       57600

NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 

// Define pins
// BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define BLUE 48

// GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
#define GOLD 49

/**
 * Get and put routines that 
 * use a test memory space.
 */

// next lines get "warning: only initialized variables can be placed into program memory area" due to GCC bug
extern "C" {
const prog_char configDefInfo[] PROGMEM = {
   60, 63, 120, 109, 108, 32, 118, 101, 114, 115, 105, 111, 110, 61, 39, 49, 46, 48, 39, 63, 62, 10, 60, 63, 120, 109, 108, 45, 115, 116, 121, 108, 101, 115, 104, 101, 101, 116, 32, 116, 121, 112, 101, 61, 39, 116, 101, 120, 116, 47, 120, 115, 108, 39, 32, 104, 114, 101, 102, 61, 39, 104, 116, 116, 112, 58, 47, 47, 111, 112, 101, 110, 108, 99, 98, 46, 111, 114, 103, 47, 116, 114, 117, 110, 107, 47, 112, 114, 111, 116, 111, 116, 121, 112, 101, 115, 47, 120, 109, 108, 47, 120, 115, 108, 116, 47, 99, 100, 105, 46, 120, 115, 108, 39, 63, 62, 10, 60, 99, 100, 105, 32, 120, 109, 108, 110, 115, 58,    // | <?xml version='1.0'?><?xml-stylesheet type='text/xsl' href='http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl'?><cdi xmlns:|
   120, 115, 105, 61, 39, 104, 116, 116, 112, 58, 47, 47, 119, 119, 119, 46, 119, 51, 46, 111, 114, 103, 47, 50, 48, 48, 49, 47, 88, 77, 76, 83, 99, 104, 101, 109, 97, 45, 105, 110, 115, 116, 97, 110, 99, 101, 39, 32, 120, 115, 105, 58, 110, 111, 78, 97, 109, 101, 115, 112, 97, 99, 101, 83, 99, 104, 101, 109, 97, 76, 111, 99, 97, 116, 105, 111, 110, 61, 39, 104, 116, 116, 112, 58, 47, 47, 111, 112, 101, 110, 108, 99, 98, 46, 111, 114, 103, 47, 116, 114, 117, 110, 107, 47, 112, 114, 111, 116, 111, 116, 121, 112, 101, 115, 47, 120, 109, 108, 47, 115, 99, 104, 101, 109, 97, 47, 99, 100,    // | xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cd|
   105, 46, 120, 115, 100, 39, 62, 10, 10, 60, 105, 100, 101, 110, 116, 105, 102, 105, 99, 97, 116, 105, 111, 110, 62, 10, 32, 32, 32, 32, 60, 109, 97, 110, 117, 102, 97, 99, 116, 117, 114, 101, 114, 62, 79, 112, 101, 110, 76, 67, 66, 60, 47, 109, 97, 110, 117, 102, 97, 99, 116, 117, 114, 101, 114, 62, 10, 32, 32, 32, 32, 60, 109, 111, 100, 101, 108, 62, 79, 108, 99, 98, 73, 111, 60, 47, 109, 111, 100, 101, 108, 62, 10, 32, 32, 32, 32, 60, 104, 97, 114, 100, 119, 97, 114, 101, 86, 101, 114, 115, 105, 111, 110, 62, 49, 46, 48, 60, 47, 104, 97, 114, 100, 119, 97, 114, 101, 86,    // | i.xsd'><identification>    <manufacturer>OpenLCB</manufacturer>    <model>OlcbIo</model>    <hardwareVersion>1.0</hardwareV|
   101, 114, 115, 105, 111, 110, 62, 10, 32, 32, 32, 32, 60, 115, 111, 102, 116, 119, 97, 114, 101, 86, 101, 114, 115, 105, 111, 110, 62, 48, 46, 52, 60, 47, 115, 111, 102, 116, 119, 97, 114, 101, 86, 101, 114, 115, 105, 111, 110, 62, 10, 60, 47, 105, 100, 101, 110, 116, 105, 102, 105, 99, 97, 116, 105, 111, 110, 62, 10, 10, 60, 115, 101, 103, 109, 101, 110, 116, 32, 111, 114, 105, 103, 105, 110, 61, 39, 48, 39, 32, 115, 112, 97, 99, 101, 61, 39, 50, 53, 51, 39, 62, 10, 32, 32, 32, 32, 60, 103, 114, 111, 117, 112, 32, 111, 102, 102, 115, 101, 116, 61, 39, 54, 54, 52, 39, 62, 10,    // | ersion>    <softwareVersion>0.4</softwareVersion></identification><segment origin='0' space='253'>    <group offset='664'>|
   32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 85, 115, 101, 114, 32, 73, 100, 101, 110, 116, 105, 102, 105, 99, 97, 116, 105, 111, 110, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 100, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 62, 76, 101, 116, 115, 32, 116, 104, 101, 32, 117, 115, 101, 114, 32, 97, 100, 100, 32, 104, 105, 115, 32, 111, 119, 110, 32, 100, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 60, 47, 100, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 115, 116, 114, 105, 110,    // |         <name>User Identification</name>        <description>Lets the user add his own description</description>        <strin|
   103, 32, 115, 105, 122, 101, 61, 39, 50, 48, 39, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 78, 111, 100, 101, 32, 78, 97, 109, 101, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 115, 116, 114, 105, 110, 103, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 115, 116, 114, 105, 110, 103, 32, 115, 105, 122, 101, 61, 39, 50, 52, 39, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 78, 111, 100, 101, 32, 68, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 60,    // | g size='20'>            <name>Node Name</name>        </string>        <string size='24'>            <name>Node Description<|
   47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 115, 116, 114, 105, 110, 103, 62, 10, 32, 32, 32, 32, 60, 47, 103, 114, 111, 117, 112, 62, 10, 60, 47, 115, 101, 103, 109, 101, 110, 116, 62, 10, 10, 60, 115, 101, 103, 109, 101, 110, 116, 32, 111, 114, 105, 103, 105, 110, 61, 39, 49, 50, 39, 32, 115, 112, 97, 99, 101, 61, 39, 50, 53, 51, 39, 62, 10, 32, 32, 32, 32, 60, 103, 114, 111, 117, 112, 32, 114, 101, 112, 108, 105, 99, 97, 116, 105, 111, 110, 61, 39, 56, 39, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 79, 117,    // | /name>        </string>    </group></segment><segment origin='12' space='253'>    <group replication='8'>        <name>Ou|
   116, 112, 117, 116, 115, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 101, 112, 110, 97, 109, 101, 62, 79, 117, 116, 112, 117, 116, 60, 47, 114, 101, 112, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 115, 116, 114, 105, 110, 103, 32, 115, 105, 122, 101, 61, 39, 49, 56, 39, 32, 111, 102, 102, 115, 101, 116, 61, 39, 51, 50, 48, 39, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 68, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32,    // | tputs</name>        <repname>Output</repname>        <string size='18' offset='320'>            <name>Description</name>    |
   32, 32, 32, 32, 60, 47, 115, 116, 114, 105, 110, 103, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 101, 118, 101, 110, 116, 105, 100, 32, 111, 102, 102, 115, 101, 116, 61, 39, 45, 51, 51, 54, 39, 32, 62, 60, 33, 45, 45, 32, 102, 111, 114, 119, 97, 114, 100, 32, 51, 50, 48, 44, 32, 97, 100, 100, 32, 49, 56, 44, 32, 115, 111, 32, 110, 111, 119, 32, 98, 97, 99, 107, 32, 51, 51, 56, 44, 32, 116, 104, 101, 110, 32, 102, 111, 114, 119, 97, 114, 100, 32, 50, 45, 45, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 83, 101, 116,    // |     </string>        <eventid offset='-336' ><!-- forward 320, add 18, so now back 338, then forward 2-->            <name>Set|
   32, 69, 118, 101, 110, 116, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 101, 118, 101, 110, 116, 105, 100, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 101, 118, 101, 110, 116, 105, 100, 32, 111, 102, 102, 115, 101, 116, 61, 39, 50, 39, 32, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 82, 101, 115, 101, 116, 32, 69, 118, 101, 110, 116, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 101, 118, 101, 110, 116, 105, 100, 62, 10, 32, 32, 32, 32, 60, 47, 103, 114, 111,    // |  Event</name>        </eventid>        <eventid offset='2' >            <name>Reset Event</name>        </eventid>    </gro|
   117, 112, 62, 10, 32, 32, 32, 32, 60, 103, 114, 111, 117, 112, 32, 114, 101, 112, 108, 105, 99, 97, 116, 105, 111, 110, 61, 39, 56, 39, 32, 111, 102, 102, 115, 101, 116, 61, 39, 50, 39, 62, 32, 32, 60, 33, 45, 45, 32, 50, 32, 102, 111, 114, 32, 101, 110, 100, 32, 111, 102, 32, 108, 97, 115, 116, 32, 101, 118, 101, 110, 116, 32, 45, 45, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 73, 110, 112, 117, 116, 115, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 101, 112, 110, 97, 109, 101, 62, 73, 110, 112, 117, 116, 60,    // | up>    <group replication='8' offset='2'>  <!-- 2 for end of last event -->        <name>Inputs</name>        <repname>Input<|
   47, 114, 101, 112, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 115, 116, 114, 105, 110, 103, 32, 115, 105, 122, 101, 61, 39, 49, 54, 39, 32, 111, 102, 102, 115, 101, 116, 61, 39, 51, 50, 48, 39, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 68, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 115, 116, 114, 105, 110, 103, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 101, 118, 101, 110, 116, 105, 100, 32, 111, 102, 102, 115, 101, 116,    // | /repname>        <string size='16' offset='320'>            <name>Description</name>        </string>        <eventid offset|
   61, 39, 45, 51, 51, 54, 39, 32, 62, 60, 33, 45, 45, 32, 102, 111, 114, 119, 97, 114, 100, 32, 51, 50, 48, 44, 32, 97, 100, 100, 32, 49, 56, 44, 32, 115, 111, 32, 110, 111, 119, 32, 98, 97, 99, 107, 32, 51, 51, 56, 44, 32, 116, 104, 101, 110, 32, 102, 111, 114, 119, 97, 114, 100, 32, 50, 45, 45, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 65, 99, 116, 105, 118, 97, 116, 105, 111, 110, 32, 69, 118, 101, 110, 116, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 101, 118, 101, 110, 116, 105,    // | ='-336' ><!-- forward 320, add 18, so now back 338, then forward 2-->            <name>Activation Event</name>        </eventi|
   100, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 101, 118, 101, 110, 116, 105, 100, 32, 111, 102, 102, 115, 101, 116, 61, 39, 50, 39, 32, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 73, 110, 97, 99, 116, 105, 118, 97, 116, 105, 111, 110, 32, 69, 118, 101, 110, 116, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 101, 118, 101, 110, 116, 105, 100, 62, 10, 32, 32, 32, 32, 60, 47, 103, 114, 111, 117, 112, 62, 10, 60, 47, 115, 101, 103, 109, 101, 110, 116, 62, 10, 10, 60, 115, 101, 103, 109, 101, 110,    // | d>        <eventid offset='2' >            <name>Inactivation Event</name>        </eventid>    </group></segment><segmen|
   116, 32, 111, 114, 105, 103, 105, 110, 61, 39, 48, 39, 32, 115, 112, 97, 99, 101, 61, 39, 50, 53, 51, 39, 62, 10, 32, 32, 32, 32, 60, 105, 110, 116, 32, 115, 105, 122, 101, 61, 39, 52, 39, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 110, 97, 109, 101, 62, 82, 101, 115, 101, 116, 60, 47, 110, 97, 109, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 100, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 62, 67, 111, 110, 116, 114, 111, 108, 115, 32, 114, 101, 108, 111, 97, 100, 105, 110, 103, 32, 97, 110, 100, 32, 99, 108, 101, 97, 114, 105, 110, 103, 32, 110, 111, 100,    // | t origin='0' space='253'>    <int size='4'>        <name>Reset</name>        <description>Controls reloading and clearing nod|
   101, 32, 109, 101, 109, 111, 114, 121, 46, 32, 66, 111, 97, 114, 100, 32, 109, 117, 115, 116, 32, 98, 101, 32, 114, 101, 115, 116, 97, 114, 116, 101, 100, 32, 102, 111, 114, 32, 116, 104, 105, 115, 32, 116, 111, 32, 116, 97, 107, 101, 32, 101, 102, 102, 101, 99, 116, 46, 60, 47, 100, 101, 115, 99, 114, 105, 112, 116, 105, 111, 110, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 109, 97, 112, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 101, 108, 97, 116, 105, 111, 110, 62, 60, 112, 114, 111, 112, 101, 114, 116, 121, 62, 51, 57, 57, 56, 53, 55, 50, 50, 54,    // | e memory. Board must be restarted for this to take effect.</description>        <map>            <relation><property>399857226|
   49, 60, 47, 112, 114, 111, 112, 101, 114, 116, 121, 62, 60, 118, 97, 108, 117, 101, 62, 40, 78, 111, 32, 114, 101, 115, 101, 116, 41, 60, 47, 118, 97, 108, 117, 101, 62, 60, 47, 114, 101, 108, 97, 116, 105, 111, 110, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 101, 108, 97, 116, 105, 111, 110, 62, 60, 112, 114, 111, 112, 101, 114, 116, 121, 62, 51, 57, 57, 56, 53, 54, 49, 50, 50, 56, 60, 47, 112, 114, 111, 112, 101, 114, 116, 121, 62, 60, 118, 97, 108, 117, 101, 62, 85, 115, 101, 114, 32, 99, 108, 101, 97, 114, 58, 32, 78, 101, 119, 32, 100, 101, 102,    // | 1</property><value>(No reset)</value></relation>            <relation><property>3998561228</property><value>User clear: New def|
   97, 117, 108, 116, 32, 69, 118, 101, 110, 116, 73, 68, 115, 44, 32, 98, 108, 97, 110, 107, 32, 115, 116, 114, 105, 110, 103, 115, 60, 47, 118, 97, 108, 117, 101, 62, 60, 47, 114, 101, 108, 97, 116, 105, 111, 110, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 101, 108, 97, 116, 105, 111, 110, 62, 60, 112, 114, 111, 112, 101, 114, 116, 121, 62, 48, 60, 47, 112, 114, 111, 112, 101, 114, 116, 121, 62, 60, 118, 97, 108, 117, 101, 62, 77, 102, 103, 32, 99, 108, 101, 97, 114, 58, 32, 82, 101, 115, 101, 116, 32, 97, 108, 108, 44, 32, 105, 110, 99, 108, 117, 100, 105,    // | ault EventIDs, blank strings</value></relation>            <relation><property>0</property><value>Mfg clear: Reset all, includi|
   110, 103, 32, 78, 111, 100, 101, 32, 73, 68, 60, 47, 118, 97, 108, 117, 101, 62, 60, 47, 114, 101, 108, 97, 116, 105, 111, 110, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 109, 97, 112, 62, 10, 32, 32, 32, 32, 60, 47, 105, 110, 116, 62, 10, 60, 47, 115, 101, 103, 109, 101, 110, 116, 62, 10, 10, 60, 47, 99, 100, 105, 62, 10,    // | ng Node ID</value></relation>        </map>    </int></segment></cdi>|
   0
};

const prog_char SNII_const_data[] PROGMEM = "\001OpenLCB\000OlcbIo\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

} // end extern "C"

#define SNII_var_data 664   // address where CDI stores SNI var strings
#define SNII_var_offset 20

#include "OlcbArduinoCAN.h"

/* *************************************************
 * EEPROM memory layout
 *     See NodeMemory.h for background info
 *
 * Internal data, not to be reset by user:
 *     0 - 3        Memory state flag
 *     4 - 5        Cycle count
 *     6 - 11       Node ID
 *
 * User configuration data:
 *     12 - 12+2*2*8*sizeof(Event) = 331  EventID storage  (Event is 10 bytes)
 *
 *     332 - 663  Event description strings
 *
 *     664 - 683     Node name (zero-term string, 20 bytes total)
 *     684 - 707     User comment (zero-term string, 24 bytes total)
 *
 *************************************************** */

#define LAST_EEPROM 707

const uint8_t getRead(uint32_t address, int space) {
  if (space == 0xFF) {
    // Configuration definition information
    return pgm_read_byte(configDefInfo+address);
  } else if (space == 0xFE) {
    // All memory reads from RAM starting at first location in this program
    return *(((uint8_t*)&rxBuffer)+address);
  } else if (space == 0xFD) {
    // Configuration space is entire EEPROM
    return EEPROM.read(address);
  } else if (space == 0xFC) { // 
    // used by ADCDI/SNII for constant data
    return pgm_read_byte(SNII_const_data+address);
  } else if (space == 0xFB) { // 
    // used by ADCDI/SNII for variable data
    return EEPROM.read(SNII_var_data+address);
  } else {
    // unknown space
    return 0; 
  }
}
void getWrite(uint32_t address, int space, uint8_t val) {
  if (space == 0xFE) {
    // All memory
    *(((uint8_t*)&rxBuffer)+address) = val;
  } else if (space == 0xFD) {
    // Configuration space
    EEPROM.write(address, val);
  } 
  // all other spaces not written
}

extern "C" {
uint8_t protocolIdentValue[6] = {0xD7,0x58,0x00,0,0,0};
    // PIP, Datagram, MemConfig, P/C, ident, teach/learn, 
    // ACDI, SNIP, CDI
}

// Events this node can produce or consume, used by PCE and loaded from EEPROM by NM
Event events[] = {
    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event(), 

    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event(), 
    Event(), Event(), Event(), Event() 
};
int eventNum = 32;

// output drivers
// 14, 15, 16, 17 for LEDuino with standard shield (set active LOW)
// 16, 17, 18, 19 for IOduino to clear built-in blue and gold
// Io 0-7 are outputs & LEDs, 8-15 are inputs (set active HIGH)
ButtonLed pOut1(0, HIGH); 
ButtonLed pOut2(1, HIGH);
ButtonLed pOut3(2, HIGH);
ButtonLed pOut4(3, HIGH);
ButtonLed pOut5(4, HIGH);
ButtonLed pOut6(5, HIGH);
ButtonLed pOut7(6, HIGH);
ButtonLed pOut8(7, HIGH);

ButtonLed pIn1(8, HIGH);
ButtonLed pIn2(9, HIGH);
ButtonLed pIn3(10, HIGH);
ButtonLed pIn4(11, HIGH);
ButtonLed pIn5(12, HIGH);
ButtonLed pIn6(13, HIGH);
ButtonLed pIn7(14, HIGH);
ButtonLed pIn8(15, HIGH);


#define ShortInpBlinkOn   0x00110011L
#define ShortInpBlinkOff  0xFFEEFFEEL
#define ShortOutBlinkOn   0x00010001L
#define ShortOutBlinkOff  0xFFFEFFFEL

long patterns[] = { // two per input or output
  ShortOutBlinkOff,ShortOutBlinkOn,
  ShortOutBlinkOff,ShortOutBlinkOn,
  ShortOutBlinkOff,ShortOutBlinkOn,
  ShortOutBlinkOff,ShortOutBlinkOn,

  ShortOutBlinkOff,ShortOutBlinkOn,
  ShortOutBlinkOff,ShortOutBlinkOn,
  ShortOutBlinkOff,ShortOutBlinkOn,
  ShortOutBlinkOff,ShortOutBlinkOn,

  ShortInpBlinkOff,ShortInpBlinkOn,
  ShortInpBlinkOff,ShortInpBlinkOn,
  ShortInpBlinkOff,ShortInpBlinkOn,
  ShortInpBlinkOff,ShortInpBlinkOn,

  ShortInpBlinkOff,ShortInpBlinkOn,
  ShortInpBlinkOff,ShortInpBlinkOn,
  ShortInpBlinkOff,ShortInpBlinkOn,
  ShortInpBlinkOff,ShortInpBlinkOn
};

ButtonLed* buttons[] = { // One for each event
                        &pOut1, &pOut1, &pOut2, &pOut2, &pOut3, &pOut3, &pOut4, &pOut4,
                        &pOut5, &pOut5, &pOut6, &pOut6, &pOut7, &pOut7, &pOut8, &pOut8,
                        &pIn1, &pIn1, &pIn2, &pIn2, &pIn3, &pIn3, &pIn4, &pIn4, 
                        &pIn5, &pIn5, &pIn6, &pIn6, &pIn7, &pIn7, &pIn8, &pIn8
                       };

ButtonLed* indicators[] = { // One for each event, remapping for BG indications (inputs and outputs same place)
                        &pOut1, &pOut1, &pOut2, &pOut2, &pOut3, &pOut3, &pOut4, &pOut4,
                        &pOut5, &pOut5, &pOut6, &pOut6, &pOut7, &pOut7, &pOut8, &pOut8,
                        &pOut1, &pOut1, &pOut2, &pOut2, &pOut3, &pOut3, &pOut4, &pOut4,
                        &pOut5, &pOut5, &pOut6, &pOut6, &pOut7, &pOut7, &pOut8, &pOut8
                       };

ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

void pceCallback(int index){
  // invoked when an event is consumed; drive pins as needed
  // from index
  //
  // sample code uses inverse of low bit of pattern to drive pin all on or all off
  // (pattern is mostly one way, blinking the other, hence inverse)
  //
  buttons[index]->on(patterns[index]&0x1 ? 0x0L : ~0x0L );
}

NodeMemory nm(0);  // allocate from start of EEPROM
void store() { nm.store(&nodeid, events, eventNum); }

PCE pce(events, eventNum, &txBuffer, &nodeid, pceCallback, store, &link);

// Set up Blue/Gold configuration

BG bg(&pce, indicators, patterns, eventNum, &blue, &gold, &txBuffer);

bool states[] = {  // one per input and output
                      false, false, false, false,
                      false, false, false, false,
                      
                      false, false, false, false,
                      false, false, false, false
                };
void produceFromInputs() {
  // called from loop(), this looks at changes in input pins (2nd half of pins, events) 
  // and decides which events to fire
  // with pce.produce(i);
  // The first event of each pair is sent on button down,
  // and second on button up.
  for (int i = eventNum/2; i<eventNum; i++) {
    if (states[i] != buttons[i*2]->state) {  // i*2 because array has doubled entries
      states[i] = buttons[i*2]->state;
      if (states[i]) {
        pce.produce(i*2);
      } else {
        pce.produce(i*2+1);
      }
    }
  }
}

/**
 * Setup does initial configuration
 */
void setup()
{
  // set up serial comm; may not be space for this!
  delay(250);Serial.begin(BAUD_RATE);logstr("\nOlcbIo\n");delay(250);
  
  // read OpenLCB from EEPROM
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state
  nm.setup(&nodeid, events, eventNum, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM);  
  
  // set event types, now that IDs have been loaded from configuration
  // newEvent arguments are (event index, produce, consume)
  for (int i=0; i<eventNum/2; i++) {
      pce.newEvent(i,false,true); // output - consumer
  }
  for (int i=eventNum/2; i<eventNum; i++) {
      pce.newEvent(i,true,false); // input - producer
  }
  
  Olcb_setup();
}

void loop() {
    bool activity = Olcb_loop();
    if (activity) {
        // blink blue to show that the frame was received
        blue.blink(0x1);
    }
    if (OpenLcb_can_active) {
        gold.blink(0x1);
        OpenLcb_can_active = false;
    }
    // handle the status lights  
    blue.process();
    gold.process();
}



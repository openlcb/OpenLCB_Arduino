

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <limits.h>
#include "gtest/gtest.h"

// deliberately do not #include "OpenLcbCan.h" to force use of numeric constants
#include "OpenLcbCanBuffer.h"
#include "ButtonLed.h"
#include "ButtonProtocol.h"

// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.
//
// <TechnicalDetails>
//
// In Google Test, tests are grouped into test cases.  This is how we
// keep test code organized.  You should put logically related tests
// into the same test case.
//
// The test case name and the test name should both be valid C++
// identifiers.  And you should not use underscore (_) in the names.
//
// Google Test guarantees that each test you define is run exactly
// once, but it makes no guarantee on the order the tests are
// executed.  Therefore, you should write your tests in such a way
// that their results don't depend on their order.
//
// </TechnicalDetails>


TEST(ButtonProtocolTest, Ctor) {
  // This test is named "Ctor", and belongs to the "ButtonProtocolTest"
  // test case.
  
  //OpenLcbCanBuffer b;
  
  ButtonLed blue(1,  LOW);
  ButtonLed gold(2,  LOW);
  
  ButtonLed pin1(10, LOW);
  ButtonLed pin2(11, LOW);
  ButtonLed pin3(12, LOW);
  
  ButtonLed* buttons[] = {&pin1,&pin1,&pin2,&pin2,&pin3,&pin3};
  
  ButtonProtocol bp(buttons, (uint8_t)6, &blue, &gold);
  
}

TEST(ButtonProtocolTest, CheckRuns) {
  
  ButtonLed blue(1,  LOW);
  ButtonLed gold(2,  LOW);
  
  ButtonLed pin1(10, LOW);
  ButtonLed pin2(11, LOW);
  ButtonLed pin3(12, LOW);
  
  ButtonLed* buttons[] = {&pin1,&pin1,&pin2,&pin2,&pin3,&pin3};
  
  ButtonProtocol bp(buttons, (uint8_t)6, &blue, &gold);
  
  bp.check();
}

TEST(ButtonProtocolTest, HandleOtherFrame) {
  
  OpenLcbCanBuffer b;
  
  ButtonLed blue(1,  LOW);
  ButtonLed gold(2,  LOW);
  
  ButtonLed pin1(10, LOW);
  ButtonLed pin2(11, LOW);
  ButtonLed pin3(12, LOW);
  
  ButtonLed* buttons[] = {&pin1,&pin1,&pin2,&pin2,&pin3,&pin3};
  
  ButtonProtocol bp(buttons, (uint8_t)6, &blue, &gold);
  
  EXPECT_FALSE(bp.receivedFrame(&b));
}

TEST(ButtonProtocolTest, HandleButtonProtocolRequestFrame) {
  
  OpenLcbCanBuffer b;
  b.setOpenLcbMTI(0x948);
  
  ButtonLed blue(1,  LOW);
  ButtonLed gold(2,  LOW);
  
  ButtonLed pin1(10, LOW);
  ButtonLed pin2(11, LOW);
  ButtonLed pin3(12, LOW);
  
  ButtonLed* buttons[] = {&pin1,&pin1,&pin2,&pin2,&pin3,&pin3};
  
  ButtonProtocol bp(buttons, (uint8_t)6, &blue, &gold);
  
  EXPECT_TRUE(bp.receivedFrame(&b));
}


TEST(ButtonProtocolTest, HandleButtonProtocolReplyFrame) {
  
  OpenLcbCanBuffer b;
  b.setOpenLcbMTI(0x949);
  
  ButtonLed blue(1,  LOW);
  ButtonLed gold(2,  LOW);
  
  ButtonLed pin1(10, LOW);
  ButtonLed pin2(11, LOW);
  ButtonLed pin3(12, LOW);
  
  ButtonLed* buttons[] = {&pin1,&pin1,&pin2,&pin2,&pin3,&pin3};
  
  ButtonProtocol bp(buttons, (uint8_t)6, &blue, &gold);
  
  EXPECT_FALSE(bp.receivedFrame(&b));
}




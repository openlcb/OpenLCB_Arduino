

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <limits.h>
#include "gtest/gtest.h"

// deliberately do not #include "OpenLcbCan.h" to force use of numeric constants
#include "OpenLcbCanBuffer.h"
#include "EventID.h"
#include "NodeID.h"

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


TEST(OpenLcbCanBufferTest, SourceAlias) {
  // This test is named "Constructor", and belongs to the "OpenLcbCanBufferTest"
  // test case.
  
  OpenLcbCanBuffer b;
  b.init(0x123);
  EXPECT_EQ(0x123, b.getSourceAlias());
  
  b.setSourceAlias(0x321);
  EXPECT_EQ(0x321, b.getSourceAlias());

}

TEST(OpenLcbCanBufferTest, FrameType) {

  OpenLcbCanBuffer b;
  
  b.setFrameTypeCAN();
  EXPECT_TRUE(b.isFrameTypeCAN());
  EXPECT_FALSE(b.isFrameTypeOpenLcb());
  
  b.setFrameTypeOpenLcb();
  EXPECT_FALSE(b.isFrameTypeCAN());
  EXPECT_TRUE(b.isFrameTypeOpenLcb());
  
  b.setFrameTypeCAN();
  EXPECT_TRUE(b.isFrameTypeCAN());
  EXPECT_FALSE(b.isFrameTypeOpenLcb());
  
  b.init(0x123);
  EXPECT_FALSE(b.isFrameTypeCAN());
  EXPECT_TRUE(b.isFrameTypeOpenLcb());

}

TEST(OpenLcbCanBufferTest, AddressedMessage) {

  OpenLcbCanBuffer b;
  b.id= 0x19008AAA;
  b.data[0] = 0x02;
  b.data[1] = 0xBE;
  b.length = 2;
  
  EXPECT_TRUE(b.isAddressedMessage());

}

TEST(OpenLcbCanBufferTest, Format) {

  OpenLcbCanBuffer b;
  b.id=0x18000000;
  
  b.setOpenLcbFormat(0x7);
  EXPECT_EQ(0x1F000000, b.id);
  EXPECT_EQ(0x7, b.getOpenLcbFormat());
  
  b.setOpenLcbFormat(0x0);
  EXPECT_EQ(0x18000000, b.id);
  EXPECT_EQ(0x0, b.getOpenLcbFormat());

  b.id=0x18FFFFFF;

  b.setOpenLcbFormat(0x7);
  EXPECT_EQ(0x1FFFFFFF, b.id);
  EXPECT_EQ(0x7, b.getOpenLcbFormat());
  
  b.setOpenLcbFormat(0x0);
  EXPECT_EQ(0x18FFFFFF, b.id);
  EXPECT_EQ(0x0, b.getOpenLcbFormat());

  b.id=0x18321123;

  b.setOpenLcbFormat(0x7);
  EXPECT_EQ(0x1F321123, b.id);
  EXPECT_EQ(0x7, b.getOpenLcbFormat());
  
  b.setOpenLcbFormat(0x0);
  EXPECT_EQ(0x18321123, b.id);
  EXPECT_EQ(0x0, b.getOpenLcbFormat());

}

TEST(OpenLcbCanBufferTest, DestAlias) {
  
  OpenLcbCanBuffer b;
  b.init(0x123);
  
  // regular MTI format
  b.setOpenLcbFormat(1);  // MTI_FORMAT_NORMAL_MTI
  b.length = 0;
  b.setDestAlias(0xFFF);
  EXPECT_EQ(0xFFF, b.getDestAlias());
  b.setDestAlias(0x000);
  EXPECT_EQ(0x000, b.getDestAlias());
  b.setDestAlias(0x543);
  EXPECT_EQ(0x543, b.getDestAlias());
  EXPECT_EQ(0x05, b.data[0]);  // location of address in frame
  EXPECT_EQ(0x43, b.data[1]);
  EXPECT_EQ(2, b.length);

  // regular Datagram formats
  b.setOpenLcbFormat(2);  // FRAME_FORMAT_ADDRESSED_DATAGRAM_ALL
  b.setDestAlias(0xFFF);
  EXPECT_EQ(0xFFF, b.getDestAlias());
  b.setDestAlias(0x000);
  EXPECT_EQ(0x000, b.getDestAlias());
  b.setDestAlias(0x543);
  EXPECT_EQ(0x543, b.getDestAlias());
  EXPECT_EQ(0x0543000, b.id&0x00FFF000);  // location of address in frame

  b.setOpenLcbFormat(3);  // FRAME_FORMAT_ADDRESSED_DATAGRAM_FIRST
  b.setDestAlias(0xFFF);
  EXPECT_EQ(0xFFF, b.getDestAlias());
  b.setDestAlias(0x000);
  EXPECT_EQ(0x000, b.getDestAlias());
  b.setDestAlias(0x543);
  EXPECT_EQ(0x543, b.getDestAlias());
  EXPECT_EQ(0x0543000, b.id&0x00FFF000);  // location of address in frame

  b.setOpenLcbFormat(4);   // FRAME_FORMAT_ADDRESSED_DATAGRAM_MID
  b.setDestAlias(0xFFF);
  EXPECT_EQ(0xFFF, b.getDestAlias());
  b.setDestAlias(0x000);
  EXPECT_EQ(0x000, b.getDestAlias());
  b.setDestAlias(0x543);
  EXPECT_EQ(0x543, b.getDestAlias());
  EXPECT_EQ(0x0543000, b.id&0x00FFF000);  // location of address in frame

  b.setOpenLcbFormat(5);   // FRAME_FORMAT_ADDRESSED_DATAGRAM_LAST
  b.setDestAlias(0xFFF);
  EXPECT_EQ(0xFFF, b.getDestAlias());
  b.setDestAlias(0x000);
  EXPECT_EQ(0x000, b.getDestAlias());
  b.setDestAlias(0x543);
  EXPECT_EQ(0x543, b.getDestAlias());
  EXPECT_EQ(0x0543000, b.id&0x00FFF000);  // location of address in frame

  // stream format
  b.setOpenLcbFormat(7);  // FRAME_FORMAT_STREAM
  b.setDestAlias(0xFFF);
  EXPECT_EQ(0xFFF, b.getDestAlias());
  b.setDestAlias(0x000);
  EXPECT_EQ(0x000, b.getDestAlias());
  b.setDestAlias(0x543);
  EXPECT_EQ(0x543, b.getDestAlias());
  EXPECT_EQ(0x0543000, b.id&0x00FFF000);  // location of address in frame

}

TEST(OpenLcbCanBufferTest, InitCompleteMessage) {
    OpenLcbCanBuffer b;
    b.init(0x123);
    NodeID n(1,2,3,4,5,6);
    EXPECT_FALSE(b.isInitializationComplete());
    
    b.setInitializationComplete(&n);
    
    EXPECT_EQ((uint32_t)0x19100123, b.id);
    EXPECT_EQ(6, b.length);
    EXPECT_EQ(0x1, b.data[0]);
    EXPECT_EQ(0x2, b.data[1]);
    EXPECT_EQ(0x3, b.data[2]);
    EXPECT_EQ(0x4, b.data[3]);
    EXPECT_EQ(0x5, b.data[4]);
    EXPECT_EQ(0x6, b.data[5]);
    
    EXPECT_TRUE(b.isInitializationComplete());
    EXPECT_TRUE(!b.isPCEventReport());
    EXPECT_TRUE(!b.isLearnEvent());
    EXPECT_TRUE(!b.isVerifiedNID());
    EXPECT_TRUE(!b.isVerifyNID());
}



TEST(OpenLcbCanBufferTest, PcerMessage) {
    OpenLcbCanBuffer b;
    b.init(0x123);
    EventID e(1,2,3,4,5,6,7,8);
    EXPECT_FALSE(b.isPCEventReport());
    
    b.setPCEventReport(&e);
    
    EXPECT_EQ((uint32_t)0x195B4123, b.id);
    EXPECT_EQ(8, b.length);
    EXPECT_EQ(0x1, b.data[0]);
    EXPECT_EQ(0x2, b.data[1]);
    EXPECT_EQ(0x3, b.data[2]);
    EXPECT_EQ(0x4, b.data[3]);
    EXPECT_EQ(0x5, b.data[4]);
    EXPECT_EQ(0x6, b.data[5]);
    EXPECT_EQ(0x7, b.data[6]);
    EXPECT_EQ(0x8, b.data[7]);
    
    EXPECT_TRUE(!b.isInitializationComplete());
    EXPECT_TRUE(b.isPCEventReport());
    EXPECT_TRUE(!b.isLearnEvent());
    EXPECT_TRUE(!b.isVerifiedNID());
    EXPECT_TRUE(!b.isVerifyNID());
}


TEST(OpenLcbCanBufferTest, LearnEventMessage) {
    OpenLcbCanBuffer b;
    b.init(0x123);
    EventID e(1,2,3,4,5,6,7,8);
    EXPECT_FALSE(b.isLearnEvent());
    
    b.setLearnEvent(&e);
    
    EXPECT_EQ((uint32_t)0x19594123, b.id);
    EXPECT_EQ(8, b.length);
    EXPECT_EQ(0x1, b.data[0]);
    EXPECT_EQ(0x2, b.data[1]);
    EXPECT_EQ(0x3, b.data[2]);
    EXPECT_EQ(0x4, b.data[3]);
    EXPECT_EQ(0x5, b.data[4]);
    EXPECT_EQ(0x6, b.data[5]);
    EXPECT_EQ(0x7, b.data[6]);
    EXPECT_EQ(0x8, b.data[7]);
    
    EXPECT_TRUE(!b.isInitializationComplete());
    EXPECT_TRUE(!b.isPCEventReport());
    EXPECT_TRUE(b.isLearnEvent());
    EXPECT_TRUE(!b.isVerifiedNID());
    EXPECT_TRUE(!b.isVerifyNID());
}

TEST(OpenLcbCanBufferTest, VerifiedNidMessage) {
    OpenLcbCanBuffer b;
    b.init(0x123);
    NodeID n(1,2,3,4,5,6);
    EXPECT_FALSE(b.isVerifiedNID());
    
    b.setVerifiedNID(&n);
    
    EXPECT_EQ((uint32_t)0x19170123, b.id);
    EXPECT_EQ(6, b.length);
    EXPECT_EQ(0x1, b.data[0]);
    EXPECT_EQ(0x2, b.data[1]);
    EXPECT_EQ(0x3, b.data[2]);
    EXPECT_EQ(0x4, b.data[3]);
    EXPECT_EQ(0x5, b.data[4]);
    EXPECT_EQ(0x6, b.data[5]);
    
    EXPECT_TRUE(!b.isInitializationComplete());
    EXPECT_TRUE(!b.isPCEventReport());
    EXPECT_TRUE(!b.isLearnEvent());
    EXPECT_TRUE(b.isVerifiedNID());
    EXPECT_TRUE(!b.isVerifyNID());
}

TEST(OpenLcbCanBufferTest, VerifyNidMessage) {
    OpenLcbCanBuffer b;
    b.init(0x123);
    NodeID n(1,2,3,4,5,6);
    EXPECT_FALSE(b.isVerifyNID());
    
    // construct manually for testing
    // note two forms
    b.setOpenLcbMTI(0x488);
    EXPECT_TRUE(b.isVerifyNID());
    b.setOpenLcbMTI(0x490);
    EXPECT_TRUE(b.isVerifyNID());
    
    EXPECT_TRUE(!b.isInitializationComplete());
    EXPECT_TRUE(!b.isPCEventReport());
    EXPECT_TRUE(!b.isLearnEvent());
    EXPECT_TRUE(!b.isVerifiedNID());
    EXPECT_TRUE(b.isVerifyNID());
}

TEST(OpenLcbCanBufferTest, OptionalIntRejectedMessage) {
    OpenLcbCanBuffer b;
    b.init(0x123);

    // sample message that's being rejected.
    EventID e(1,2,3,4,5,6,7,8);    
    OpenLcbCanBuffer request;
    request.init(0xABC);
    request.setLearnEvent(&e);
        
    // construct manually for testing
    // note two forms
    b.setOptionalIntRejected(&request, 0x8765);
    
    EXPECT_TRUE(b.isOpenLcbMTI(0x068));
    EXPECT_EQ(0x068, b.getOpenLcbMTI());
    EXPECT_EQ(0x0A, b.data[0]);
    EXPECT_EQ(0xBC, b.data[1]);  // address
    EXPECT_EQ(0x87, b.data[2]);
    EXPECT_EQ(0x65, b.data[3]);  // code
    EXPECT_EQ(0x05, b.data[4]);
    EXPECT_EQ(0x94, b.data[5]);  // MTI


    EXPECT_TRUE(!b.isInitializationComplete());
    EXPECT_TRUE(!b.isPCEventReport());
    EXPECT_TRUE(!b.isLearnEvent());
    EXPECT_TRUE(!b.isVerifiedNID());
    EXPECT_TRUE(!b.isVerifyNID());
}

TEST(OpenLcbCanBufferTest, IdentifyConsumers) {
    OpenLcbCanBuffer b;
    b.init(0x123);
    EventID e(1,2,3,4,5,6,7,8);
    EXPECT_FALSE(b.isIdentifyConsumers());
    
    b.id = 0x198F4AAA;

    EXPECT_TRUE(b.isIdentifyConsumers());
      
    EXPECT_TRUE(!b.isInitializationComplete());
    EXPECT_TRUE(!b.isPCEventReport());
    EXPECT_TRUE(!b.isLearnEvent());
    EXPECT_TRUE(!b.isVerifiedNID());
    EXPECT_TRUE(!b.isVerifyNID());
}

TEST(OpenLcbCanBufferTest, IdentifyProducers) {
    OpenLcbCanBuffer b;
    b.init(0x123);
    EventID e(1,2,3,4,5,6,7,8);
    EXPECT_FALSE(b.isIdentifyProducers());
    
    b.id = 0x19914AAA;

    EXPECT_TRUE(b.isIdentifyProducers());
      
    EXPECT_TRUE(!b.isInitializationComplete());
    EXPECT_TRUE(!b.isPCEventReport());
    EXPECT_TRUE(!b.isLearnEvent());
    EXPECT_TRUE(!b.isVerifiedNID());
    EXPECT_TRUE(!b.isVerifyNID());
}


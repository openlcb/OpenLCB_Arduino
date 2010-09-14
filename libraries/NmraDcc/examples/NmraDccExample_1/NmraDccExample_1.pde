#include <NmraDcc.h>

NmraDcc  Dcc ;
DCC_MSG  Packet ;

const int DccAckPin = 3 ;

  // This function is called by the NmraDcc library when a DCC ACK needs to be sent
void notifyCVAck(void)
{
  Serial.println("notifyCVAck") ;
  delay( 6 );  
  digitalWrite( DccAckPin, LOW );
}

void notifyDccSigState( uint16_t Addr, uint8_t OutputIndex, uint8_t State)
{
  Serial.print("notifyDccSigState: ") ;
  Serial.print(Addr) ;
  Serial.print(',');
  Serial.print(OutputIndex) ;
  Serial.print(',');
  Serial.println(State, HEX) ;
}


void setup()
{
  Serial.begin(115200);
  
    // Configure the DCC CV Programing ACK pin for an output 
  pinMode( DccAckPin, OUTPUT );

    // Enable the Pull-Up on the DCC Signal Input pin INT0 
  digitalWrite(2, HIGH);

    // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init( MAN_ID_DIY, 10, FLAGS_OUTPUT_ADDRESS_MODE | FLAGS_DCC_ACCESSORY_DECODER, 33 );
}

void loop()
{
  Dcc.process();
}


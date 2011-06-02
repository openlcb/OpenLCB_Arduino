#include <OLCB_AliasCache.h>
#include <OLCB_Buffer.h>
#include <OLCB_CAN_Buffer.h>
#include <OLCB_Link.h>
#include <OLCB_CAN_Link.h>
#include <OLCB_Datagram.h>
#include <OLCB_Datagram_Handler.h>
#include <OLCB_Event.h>
#include <OLCB_EventID.h>
#include <OLCB_Handler.h>
#include <OLCB_NodeID.h>
#include <OLCB_Stream.h>

#include <can.h>

class virtualNodeFactory;

class virtualNode: public OLCB_Datagram_Handler
{
  friend class virtualNodeFactory;

 public:
  bool processDatagram(void) //NOT "boolean"!
  {
     //To have made it this far, we can be sure that _rxDatagramBuffer has a valid datagram loaded up, and that it is in fact addressed to us.
     Serial.println("Received a datagram!");
     for(int i = 0; i < _rxDatagramBuffer->length; ++i)
     {
       Serial.print("    ");
       Serial.println(_rxDatagramBuffer->data[i], HEX);
     }
     return true; //returning true causes an ACK; returning false a NAK. Not very sophisticated yet...
  }
  
  void update(void)
  {
    OLCB_Datagram_Handler::update();
  }
  
  boolean _announced;
  boolean _ready;
};

class virtualNodeFactory: public OLCB_Datagram_Handler
{
 public:
  void init(void)
  {
    Serial.println("Initializing");
    for(int i = 0; i < 10; ++i)
    {
      Serial.println(i,DEC);
      nodes[i] = (virtualNode*)malloc(sizeof(virtualNode));
      nodes[i]->_initialized = false;
      nodes[i]->_announced = false;
      nodes[i]->_ready = false;
      nodes[i]->NID = (OLCB_NodeID*)malloc(sizeof(OLCB_NodeID));
      nodes[i]->_rxDatagramBuffer->destination = nodes[i]->NID;
      nodes[i]->_txDatagramBuffer->source = nodes[i]->NID;
      nodes[i]->next = NULL;
    }
    Serial.println("Done initializing");
  }
  
   //intercept verifyNID messages intended for locomotives, and try to create a new virtual node, if possible
   //notice that because of the order the various handlers get added to link, this method will only ever be
   //called if none of the extant virtualNodes have already handled it. So we know going into it that there
   //isn't already a virtual node at this address.
  bool verifyNID(OLCB_NodeID *nid)
  {
    if( (nid->val[0] == 6) && (nid->val[1] == 1) ) //if it's intended for a DCC locomotive
    {
      Serial.println("Producing a new virtual node for address: ");
      nid->print();
      //find a slot for it
      Serial.println(nodes[2]->_ready,DEC);
      for(int i = 0; i < 10; ++i)
      {
        if(!(nodes[i]->_ready)) //an empty slot is found!
        {
          Serial.print("    Installing in slot ");
          Serial.println(i,DEC);
          //bypassing the usual methods for doing this.
          Serial.println(nodes[2]->_ready,DEC);
          memcpy(nodes[i]->NID,nid, sizeof(OLCB_NodeID));
          nodes[i]->_ready = true;
          return false; //what the what? we're actually not yet ready to send out the verifiedNID packet!
        }
        else
        {  
          Serial.print("   Slot taken: ");
          Serial.println(i,DEC);
        }
      }
      Serial.println("    Out of slots. Too bad.");
    }
    return false; //no room availble, or not a request for a loco.
  }
  
  void completeProduction(void)
  {
    //here we will make sure that new nodes are up to date. The problem is that we cannot call virtualNode::setLink()
    //from within any method called by link. The reason is that we aren't allowed to muck with the link list maintained
    //by link. There should be a better way to handle this.
    for(int i = 0; i < 10; ++i)
    {
      if(nodes[i] != NULL) //if there is something here
      {
        if(!nodes[i]->_link)
        {
          Serial.print("virtual node ");
          Serial.print(i,DEC);
          Serial.println(" requires initialization");
          Serial.println((uint16_t)(nodes[i]),DEC);
          nodes[i]->_link = _link;
          _link->addHandler(nodes[i]);
          //next time update() is called, the virtualNode will observe that it hasn't been registered, and will register itself.
        }
        else if(nodes[i]->_initialized && !nodes[i]->_announced)
        {
          Serial.print("virtual node ");
          Serial.print(i,DEC);
          Serial.println(" requires verification");
          nodes[i]->_announced = true;
          //send out a verifiedID message, because someone is waiting on it.
          OLCB_CAN_Link* temp = (OLCB_CAN_Link*)_link;
          temp->sendVerifiedNID(nodes[i]->NID);
        }
      }
    }
  }
  
  virtualNode *nodes[10];
};

virtualNodeFactory locoFactory;

OLCB_NodeID nid(2,1,13,0,0,2);
OLCB_CAN_Link link(&nid);

void setup() {
  Serial.begin(115200);
  Serial.println("Begin!");
  delay(1000);
  // put your setup code here, to run once:
  link.initialize();

  Serial.print("This is my alias (should not be 0): ");
  Serial.println(nid.alias);
  locoFactory.setLink((OLCB_Link*)&link);
  locoFactory.init();
}

uint8_t * heapptr, * stackptr;
void check_mem() {
  stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);      // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);           // save value of stack pointer
}


void loop() {
  link.update();
//  check_mem();
//  Serial.println(stackptr - heapptr, DEC);
//  locoFactory.completeProduction();
}

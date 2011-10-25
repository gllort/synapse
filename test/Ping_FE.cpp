#include <iostream>
#include "Ping_FE.h"

/** 
 * In the Setup function we have to register all the streams we want to use for this protocol. 
 * When the function returns, all streams pushed to the registeredStreams queue are automatically
 * published to the back-ends. The Register_Stream is a wrapper to net->new_Stream() that 
 * creates a new stream in the FE and pushes it to the queue.
 */
void Ping::Setup()
{
   stAdd = Register_Stream(TFILTER_SUM, SFILTER_WAITFORALL);
   cout << "[FE] Created new stream " << stAdd->get_Id() << endl;
}

/**
 * Implement the front-end side of the protocol. 
 * It is expected to return 0 on success; -1 otherwise.
 */
int Ping::Run()
{
   int tag, countPongs = 0;
   PacketPtr p;

   cout << "[FE] Sending PING to " << stAdd->size() << " back-ends through stream " << stAdd->get_Id() << endl;
   MRN_STREAM_SEND(stAdd, TAG_PING, "");
   cout << "[FE] Waiting for PONG from " << stAdd->size() << " back-ends..." << endl;
   MRN_STREAM_RECV(stAdd, &tag, p, TAG_PONG);
   p->unpack("%d", &countPongs);
   cout << "[FE] " << countPongs << " PONGs received!" << endl;

   if (countPongs == stAdd->size())
   {
      cout << "[FE] Addition filter ran successfully! :)" << endl;
      return 0;
   }
   else
   {
      cout << "[FE] Addition filter FAILED! :(" << endl;
      return -1;
   }
}


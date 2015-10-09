#include <iostream>
#include "Ping_BE.h"

/**
 * The streams created in the front-end are received here when Register_Streams() is called, in the same order that were created.
 */
void Ping::Setup()
{
   Register_Stream(stAdd);
}

/**
 * Implement the back-end side of the protocol.
 * It is expected to return 0 on success; -1 otherwise.
 */
int Ping::Run()
{
   int tag;
   PACKET_new(p);

   MRN_STREAM_RECV(stAdd, &tag, p, TAG_PING);
   MRN_STREAM_SEND(stAdd, TAG_PONG, "%d", 1);

   PACKET_delete(p);

   return 0;
}


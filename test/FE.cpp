#include <iostream>
#include "FrontEnd.h"
#include "Ping_FE.h"

using namespace Synapse;

int main(int argc, char *argv[])
{
   /* Start the front-end side of the network */
   FrontEnd *FE = new FrontEnd();
   FE->Init("topology_1x4.txt", "./test_BE", NULL);

   /* Load the protocols the network understands */
   FrontProtocol *prot = new Ping();
   FE->LoadProtocol( prot )  ;

   /* Execute protocol "PING" */
   int status;
   FE->Dispatch("PING", status);

   /* Shutdown the network */
   FE->Shutdown();

   return 0;
}



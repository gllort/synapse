#include "BackEnd.h"
#include "Ping_BE.h"

int main(int argc, char *argv[])
{
   /* Start the back-end side of the network */
   BackEnd *BE = new BackEnd();
   BE->Init(argc, argv);

   /* Load the protocols the network understand */
   BackProtocol *prot = new Ping();
   BE->LoadProtocol(prot);

   /* The back-end enters the main analysis loop, 
      waiting for commands from the front-end */
   BE->Loop();

   return 0;
}


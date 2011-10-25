#include <iostream>
#include "FrontEnd.h"
#include "Ping_FE.h"


int main(int argc, char *argv[])
{
   FrontEnd *FE = new FrontEnd();

   FE->Init("topology_1x4.txt", "./BE", NULL);
   FrontProtocol *prot = new Ping();
   FE->LoadProtocol( prot )  ;

   Protocol *p;
   FE->Dispatch("PING", p);

   sleep(5);
   FE->Shutdown();

   return 0;
}



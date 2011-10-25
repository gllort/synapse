#include "BackEnd.h"
#include "Ping_BE.h"

int main(int argc, char *argv[])
{
   BackEnd *BE = new BackEnd();
   BE->Init(argc, argv);

   BackProtocol *prot = new Ping();
   BE->LoadProtocol(prot);


   BE->Loop();

   return 0;
}


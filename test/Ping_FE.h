#ifndef __PING_FE_H__
#define __PING_FE_H__

#include "FrontProtocol.h"
#include "tags.h"

class Ping : public FrontProtocol
{
public:
   string ID() { return "PING"; }
   void Setup();
   int  Run();
private:
   STREAM *stAdd;
};

#endif /* __PING_FE_H__ */

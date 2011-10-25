#ifndef __PING_BE_H__
#define __PING_BE_H__

#include "BackProtocol.h"
#include "tags.h"

class Ping : public BackProtocol
{
public:
   string ID() { return "PING"; }
   void Setup();
   int  Run();
private:
   STREAM *stAdd;
};

#endif /* __PING_BE_H__ */

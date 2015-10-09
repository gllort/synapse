#ifndef __PING_BE_H__
#define __PING_BE_H__

#include "BackProtocol.h"
#include "tags.h"

using namespace Synapse;

class Ping : public BackProtocol
{
   public:
      string ID (void) { return "PING"; } /* Must coincide with the front-end protocol */
      void Setup(void);
      int  Run  (void);

   private:
      STREAM *stAdd;
};

#endif /* __PING_BE_H__ */

#ifndef __PING_FE_H__
#define __PING_FE_H__

#include "FrontProtocol.h"
#include "tags.h"

class Ping : public FrontProtocol
{
   public:
      string ID (void) { return "PING"; } /* Must coincide with the back-end protocol */
      void Setup(void);
      int  Run  (void);

   private:
      STREAM *stAdd;
};

#endif /* __PING_FE_H__ */

#include "Protocol.h"
#include "MRNetApp.h"

Protocol::Protocol()
{
   mrnApp = NULL;
}

/**
 * Returns the back-end identifier.
 * @return the MRNetID of the back-end that has this protocol loaded.
 */
unsigned int Protocol::WhoAmI()
{
   return mrnApp->WhoAmI();
}


/**
 * Returns the number of back-ends in the network where this protocol is loaded.
 * @return number of back-ends.
 */
unsigned int Protocol::NumBackEnds()
{
   return mrnApp->NumBackEnds();
}


/**
 * Returns the MRNet Network object where this protocol is loaded.
 * @return the MRNet Network.
 */
NETWORK * Protocol::GetNetwork()
{
   return mrnApp->GetNetwork();
}


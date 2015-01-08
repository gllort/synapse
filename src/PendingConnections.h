#ifndef __PENDING_CONNECTIONS_H__
#define __PENDING_CONNECTIONS_H__

#include <string>
#include "MRNet_wrappers.h"

#define MAX_PARSE_RETRIES  100
#define IDLE_BETWEEN_TRIES 10

using std::string;

class PendingConnections
{
  public:
    PendingConnections(string ConnectionsFile);
 
    /* Front-end API */
    int Write(NETWORK *net, unsigned int numBackends);

    /* Back-end API */
    int GetParentInfo( int rank, char *phost, char *pport, char *prank );
    int ParseForMPIDistribution(int world_size, char *&sendbuf, int *&sendcnts, int *&displs);

  private:
    string ConnectionsFile;
};

#endif /* __PENDING_CONNECTIONS_H__ */

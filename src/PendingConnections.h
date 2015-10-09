/*****************************************************************************\
 *                              Synapse library                              *
 *               Simple interface to create MRNet applications               *
 *****************************************************************************
 *     ___          This library is free software; you can redistribute it   *
 *    /  __         and/or modify it under the terms of the GNU LGPL as pub- *
 *   /  /  _____    lished by the Free Software Foundation; either version   *
 *  /  /  /     \   2.1 of the License or (at your option) any later version.*
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 * ------------------------------------------------------------------------- *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

#ifndef __PENDING_CONNECTIONS_H__
#define __PENDING_CONNECTIONS_H__

#include <string>
#include "MRNet_wrappers.h"

#define MAX_PARSE_RETRIES  100
#define IDLE_BETWEEN_TRIES 10

using std::string;

namespace Synapse {

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

} /* namespace Synapse */

#endif /* __PENDING_CONNECTIONS_H__ */

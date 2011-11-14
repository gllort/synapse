/*****************************************************************************\
 *                             MRNetApp library                              *
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

#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <string>
#include <vector>
#include "MRNetApp.h"

#define MAX_RETRIES 60 /* Seconds to wait for the backends to connect before throwing a timeout */

using std::string;
using std::vector;

class FrontEnd : public MRNetApp
{
   public:
      unsigned int numBackendsConnected; /* public so it can be accessed by the BE Quit callback */

      FrontEnd();
      bool isFE(); 

      int  Init(const char *TopologyFile, const char *BackendExe,   const char **BackendArgs);
      int  Init(const char *BackendExe,   const char **BackendArgs);
      int  Init(const char *TopologyFile, unsigned int numBackends, const char *ConnectionsFile);
      int  Init(unsigned int numBackends, const char *ConnectionsFile);
      int  ConnectedBackEnds(void);
      int  LoadProtocol(Protocol *prot);
      int  LoadFilter  (string filter_name);
      int  Dispatch    (string protID, Protocol *& prot);
      int  Dispatch    (string protID);
      void Shutdown    (void);

   private:
      int Init();
      int WaitForBackends(unsigned int numBackends, const char *ConnectionsFile);
      int WriteConnections(
         vector< NetworkTopology::Node * >& internalLeaves,
         unsigned int                       numBackends,
         const char                        *ConnectionsFile);
};

#endif /* __FRONTEND_H__ */


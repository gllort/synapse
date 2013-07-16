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

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <queue>
#include <string>
#include "MRNet_wrappers.h"

using namespace std;

class MRNetApp;

class Protocol 
{
   public:
      Protocol();
      virtual ~Protocol() { };

      virtual void Init (MRNetApp *) = 0;

      /* The following have to be defined for every protocol implementation */
      virtual string ID   (void) = 0;  /* Returns the protocol (textual) identifier                            */
      virtual void   Setup(void) { }; /* Redefine this to register new streams using calls to Register_Stream */
      virtual int    Run  (void) = 0;  /* Implements the protocol to execute                                   */

      unsigned int WhoAmI(bool return_network_id=false);
      unsigned int NumBackEnds();
      NETWORK * GetNetwork();

   protected:
      /* Stores the streams that are created in Setup() using Register_Stream. All streams in this queue
         by the end of Setup() (in the front-end) are automatically send to the back-ends and stored here.
         Calls to Register_Stream in the back-end pop the streams from the queue and return them to the user. */
      queue<STREAM *> registeredStreams;

      MRNetApp *mrnApp;
};

#endif /* __PROTOCOL_H__ */

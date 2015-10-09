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

#include <iostream>
#include "BackProtocol.h"
#include "BackEnd.h"

using std::cerr;
using std::endl;
using namespace Synapse;


/**
 * Calls the user-defined BE initialization routine for this protocol and 
 * then receives all the streams that were created in the front-end.
 * @param be The BackEnd object.
 */
void BackProtocol::Init(MRNetApp *BE)
{
   mrnApp = BE;
   AnnounceStreams();
   Setup(); /* User-defined in the specific protocol object implementation */
}


/**
 * Retrieves a new stream that was registered in the front-end. The streams 
 * have to be registered in the same order than in the FE!
 * @param new_stream Stream that was registered in the front-end.
 */
void BackProtocol::Register_Stream(STREAM *& new_stream)
{
   new_stream = registeredStreams.front();
   /* Remove the stream from the queue */
   registeredStreams.pop();
}


/**
 * Automatically receives all the streams that were registered in the front-end. 
 * The streams are stored in the registeredStreams queue in the same order that 
 * were created. Subsequent calls to Register_Stream will fetch the streams from 
 * the queue and return them to the user. 
 * @return 0 on success; -1 otherwise.
 */
int BackProtocol::AnnounceStreams()
{
   int tag;
   PACKET_new(p);
   unsigned int countStreams = 0;

   /* Read the number of streams that were created */
   MRN_STREAM_RECV(mrnApp->stControl, &tag, p, TAG_STREAM);
   PACKET_unpack(p, "%d", &countStreams);

   /* DEBUG -- Number of streams 
   std::cout << "[BE " << WhoAmI() << "] BackProtocol::AnnounceStreams: Receiving " << countStreams << " streams" << std::endl; */

   /* Receive them 1 by 1 */
   for (unsigned int i=0; i<countStreams; i++)
   {
      STREAM *newStream;
      MRN_NETWORK_RECV(mrnApp->net, &tag, p, TAG_STREAM, &newStream, true);
      /* DEBUG
      std::cout <<  "[BE " << WhoAmI() << "] BackProtocol::AnnounceStreams: Received stream #" << newStream->get_Id() << std::endl; */
      registeredStreams.push(newStream);
   }
   /* Send reception confirmation */
   MRN_STREAM_SEND(mrnApp->stControl, TAG_ACK, "%d", 1);
   PACKET_delete(p);

   return 0;
}

int BackProtocol::Barrier()
{
  int tag;
  PACKET_new(p);
  unsigned int countACKs = 0;

  MRN_STREAM_SEND(mrnApp->stControl, TAG_ACK, "%d", 1);
  MRN_STREAM_RECV(mrnApp->stControl, &tag, p, TAG_ACK);
  PACKET_unpack(p, "%d", &countACKs);
 
  if (countACKs != mrnApp->NumBackEnds())
  {
      cerr << "[BE] " << WhoAmI() << "] ERROR: BackProtocol::Barrier: " << countACKs << " ACKs received, expected " << mrnApp->NumBackEnds() << endl;
      return -1;
  }
  return 0;
}

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

#include <iostream>
#include "FrontProtocol.h"
#include "FrontEnd.h"

using namespace std;


/**
 * Calls the user-defined FE intialization routine for this protocol and 
 * then publishes all the streams that are registered by the user.
 * @param fe The FrontEnd object
 */
void FrontProtocol::Init(MRNetApp *FE)
{
   mrnApp = FE;
   Setup(); /* User-defined in the specific protocol object implementation */
   AnnounceStreams();
}


/**
 * Wrapper for net->new_Stream that stores the newly created stream in the registration queue.
 * @param up_transfilter_id Transformation filter to apply to data flowing upstream (default is TFILTER_NULL)
 * @param up_syncfilter_id Synchronization filter to apply to upstream packets (default is SFILTER_WAITFORALL)
 * @return the new stream.
 */
STREAM * FrontProtocol::Register_Stream(int up_transfilter_id = TFILTER_NULL, int up_syncfilter_id = SFILTER_WAITFORALL)
{
   Communicator *comm_BC = mrnApp->net->get_BroadcastCommunicator();
   STREAM *new_stream = mrnApp->net->new_Stream(comm_BC, up_transfilter_id, up_syncfilter_id);
   registeredStreams.push(new_stream);
   return new_stream;
}


/**
 * Wrapper for net->new_Stream that stores the newly created stream in the registration queue.
 * The filter identified by filter_name is loaded into the network and linked to the new stream.
 * @param filter_name Transformation filter to apply to data flowing upstream 
 * @param up_syncfilter_id Synchronization filter to apply to upstream packets (default is SFILTER_WAITFORALL)
 * @return the new stream.
 */
STREAM * FrontProtocol::Register_Stream(string filter_name, int up_syncfilter_id = SFILTER_WAITFORALL)
{
   Communicator *comm_BC = mrnApp->net->get_BroadcastCommunicator();
   int filter_id = ((FrontEnd *)mrnApp)->LoadFilter( filter_name ) ;
   STREAM *new_stream = mrnApp->net->new_Stream(comm_BC, filter_id, up_syncfilter_id);
   registeredStreams.push(new_stream);
   return new_stream;
}


/**
 * Automatically publishes all the streams that are queued in 'registeredStreams' to the back-ends.
 * return 0 on success; -1 otherwise.
 */
int FrontProtocol::AnnounceStreams()
{
   int tag, NumberOfStreams=0;
   PacketPtr p;

   /* Announce streams to the back-ends */
   unsigned int countACKs = 0;

   /* DEBUG 
   std::cout << "[FE] FrontProtocol::AnnounceStreams: Sending " << registeredStreams.size() << " streams" << std::endl; */

   /* Send the number of streams */
   NumberOfStreams = registeredStreams.size();
   MRN_STREAM_SEND(mrnApp->stControl, TAG_STREAM, "%d", registeredStreams.size());

   for (unsigned int i=0; i<NumberOfStreams; i++)
   {
      STREAM *st = registeredStreams.front();
      /* DEBUG
      std::cout << "[FE] FrontProtocol::AnnounceStreams: Publishing stream #" << st->get_Id() << " streams" << std::endl; */
      /* Send a message through every stream */
      MRN_STREAM_SEND(st, TAG_STREAM, "");
      /* Remove the stream from the queue */
      registeredStreams.pop();
   }
   /* Read ACKs */
   MRN_STREAM_RECV(mrnApp->stControl, &tag, p, TAG_ACK);
   p->unpack("%d", &countACKs);
   if (countACKs != mrnApp->stControl->size())
   {
      cerr << "[FE] Error announcing streams! (" << countACKs << " ACKs received, expected " << mrnApp->stControl->size() << ")" << endl;
      return -1;
   }
   return 0;
}


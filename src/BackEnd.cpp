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
#include <assert.h>
#include <fstream>
#include "BackEnd.h"
#include "BackProtocol.h"

using namespace std;


/**
 * BackEnd constructor.
 */
BackEnd::BackEnd()
{
   WhoAmI              = 0;
   No_BE_Instantiation = false;
}


/**
 * Starts the network backend-side (normal instantiation).
 * @param argc Number of backend argumens.
 * @param argv Backend arguments.
 * @return 0 on success; -1 otherwise.
 */
int BackEnd::Init (int argc, char *argv[])
{
   net = NETWORK_CreateNetworkBE( argc, argv );
   assert(net);

   return Init();
}


/**
 * Starts the network connecting pending backends (No-BE instantiation).
 * @param wRank           Backend world rank.
 * @param connectionsFile Backends connections file.
 * @return 0 on success; -1 otherwise.
 */ 
int BackEnd::Init (int wRank, const char *connectionsFile)
{
   No_BE_Instantiation = true;

   net = Connect(wRank, connectionsFile);
   assert(net);

   return Init();
}


/** 
 * Common backend initialization receives the control stream from the frontend.
 * @return 0 on success; -1 otherwise.
 */
int BackEnd::Init()
{
   int tag;
   PACKET_new(p);

   WhoAmI = NETWORK_get_LocalRank(net);

   /* Receive the control stream */
   int rc = NETWORK_recv(net, &tag, p, &stControl, true);
   PACKET_delete(p);
   if ( rc != 1 )
   {
      cerr << "[BE " << WHOAMI << "] net::recv() failure" << endl;
      return -1;
   }
   return 0;
}


/**
 * Connects the backend to the network (remote instantiation).
 * @param wRank           Backend world rank.
 * @param connectionsFile Backends connections file.
 * @return The MRNet Network on success; NULL otherwise.
 */
NETWORK * BackEnd::Connect(int wRank, const char *connectionsFile)
{
   /* Get connection information */
   char myHostname[64], parHostname[64], parPort[10], parRank[10], myRank[10];

   while( gethostname(myHostname, 64) == -1 ) {}
   myHostname[63] = '\0';

   if (getParentInfo(connectionsFile, wRank, parHostname, parPort, parRank) == -1)
   {
      cerr << "[BE " << wRank << "] failed to parse connections file" << endl;
      return NULL;
   }

   /* Connect the BE to the network */
   cout << "BackEnd " << myHostname << "[" << wRank << "] connecting to " 
        << parHostname << ":" << parPort << endl;

   int   BE_argc = 6;
   char *BE_argv[BE_argc];
   BE_argv[0] = NULL;
   BE_argv[1] = parHostname;
   BE_argv[2] = parPort;
   BE_argv[3] = parRank;
   BE_argv[4] = myHostname;
   sprintf(myRank, "%d", MRNET_RANK(wRank));
   BE_argv[5] = myRank;

   net = NETWORK_CreateNetworkBE(BE_argc, BE_argv);

   return net;
}


/**
 * Retrieves host, port and rank where a given backend connects from the connections file.
 * @param file Backends connections file.
 * @param rank Backend rank.
 * @return Host, port and rank where this backend will connect and 0 on success; -1 otherwise.
 */
int BackEnd::getParentInfo(const char *file, int rank, char *phost, char *pport, char *prank)
{
   ifstream ifs(file);
   if( ifs.is_open() ) 
   {
      while( ifs.good() ) 
      {
         char line[256];
         ifs.getline( line, 256 );

         char pname[64];
         int tpport, tprank, trank;
         int matches = sscanf( line, "%s %d %d %d",
                               pname, &tpport, &tprank, &trank );
         if( matches != 4 ) 
         {
            fprintf(stderr, "Error while scanning %s\n", file);
            ifs.close();
            return -1;
         }
         if( trank == rank ) 
         {
            sprintf(phost, "%s", pname);
            sprintf(pport, "%d", tpport);
            sprintf(prank, "%d", tprank);
            ifs.close();
            return 0;
         }
      }
      ifs.close();
   }
   // my rank not found :(
   return -1;
}


/**
 * Loads an user-protocol for the back-end side of the MRNet.
 * @param prot Protocol to load in the back-end.
 * @return 0 on success; -1 otherwise.
 */
int BackEnd::LoadProtocol(Protocol *prot)
{
   ((BackProtocol *)prot)->Init(this);
   return MRNetApp::LoadProtocol(prot);
}


/**
 * The back-end enters a loop waiting for requests from the front-end.
 * When the FE dispatches a request, the back-end executes the counterpart
 * for the same protocol. The loop exits when TAG_EXIT is received. 
 */
void BackEnd::Loop()
{
   Protocol *prot;
   int next_tag;
   int err = 0;
   char *prot_id;
   PACKET_new(p);

   do
   {
      /* Read the next protocol request */
      MRN_STREAM_RECV(stControl, &next_tag, p, TAG_ANY);
      /* Leave the loop on TAG_EXIT */
      if (next_tag != TAG_EXIT) 
      {
         PACKET_unpack(p, "%s", &prot_id);
         /* Fetch the back-end protocol */
         prot = MRNetApp::FetchProtocol(string(prot_id));
         if (prot != NULL)
         {
            /* Execute the back-end side of the protocol */
            err = prot->Run();
         }
         /* Notify success or errors  */
         MRN_STREAM_SEND(stControl, TAG_ACK, "%d", err*(-1)); /* 0 success, +1 error */
      } 
   } while (next_tag != TAG_EXIT);

   PACKET_delete(p);
   Shutdown();
}


/**
 * Shutdown the MRNet.
 */
void BackEnd::Shutdown()
{
   /* Notify this BE is ready to exit */
   MRN_STREAM_SEND(stControl, TAG_EXIT, "%d", 1);

   /* Wait for stControl to be closed */
#if !defined(LIGHTWEIGHT)
   /* In the lightweight library streams are freed during network shutdown, 
      so there's no need to wait for the stream to be closed */
   if( stControl != NULL )
   {
      while( ! STREAM_is_Closed(stControl) ) sleep(1);
      STREAM_delete(stControl);
   }
#endif

   /* FE delete of the net will cause us to exit, wait for it */
   NETWORK_waitfor_ShutDown(net);
   NETWORK_delete(net);

   sleep(3);
}


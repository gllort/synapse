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
#include "BackEnd.h"
#include "BackProtocol.h"
#include "PendingConnections.h"
#include <sstream>

using namespace std;

/**
 * BackEnd constructor.
 */
BackEnd::BackEnd()
{
}


/**
 * Called from the superclass to identify which type of MRNet process this is.
 * @return true.
 */
bool BackEnd::isBE()
{
   return true;
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

   return CommonInit();
}


/**
 * Starts the network connecting pending backends (remote instantiation), 
 * reading the connection information from the specified file.
 * @param wRank           Backend world rank.
 * @param connectionsFile Backends connections file.
 * @return 0 on success; -1 otherwise.
 */ 
int BackEnd::Init (int wRank, const char *connectionsFile)
{
   Remote_Instantiation = true;

   net = Connect(wRank, connectionsFile);
   assert(net);

   return CommonInit();
}

/**
 * Starts the network connecting pending backends (remote instantiation), 
 * using the specified connection information to attach the back-ends.
 * This variant was implemented to avoid stressing the filesystem by 
 * reading the connections file simultaneously by many back-ends.
 * @param wRank           Backend world rank.
 * @param parHostname     Parent's hostname.
 * @param parPort         Parent's port.
 * @param parRank         Parent's rank.
 * @return 0 on success; -1 otherwise.
 */
int BackEnd::Init (int wRank, char *parHostname, int parPort, int parRank)
{
   Remote_Instantiation = true;

   stringstream ss_port, ss_rank;
   ss_port << parPort;
   ss_rank << parRank;
   net = Connect(wRank, parHostname, (char *)ss_port.str().c_str(), (char *)ss_rank.str().c_str());
   assert(net);
  
   return CommonInit();
}


/**
 * Reads the connection file from the environment variable MRNAPP_BE_CONNECTIONS and starts the network.
 * @param wRank Backend world rank.
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int BackEnd::Init (int wRank)
{
   char *env_MRNAPP_BE_CONNECTIONS = getenv("MRNAPP_BE_CONNECTIONS");
   if (env_MRNAPP_BE_CONNECTIONS == NULL)
   {
      cerr << "[BE " << wRank << "] ERROR: MRNAPP_BE_CONNECTIONS environment variable is not defined!" << endl;
      cerr << "[BE " << wRank << "] Make it point to the back-ends connection file." << endl;
      return -1;
   }
   return Init(wRank, ((const char *)env_MRNAPP_BE_CONNECTIONS));
}


/** 
 * Common backend initialization receives the control stream from the frontend.
 * @return 0 on success; -1 otherwise.
 */
int BackEnd::CommonInit()
{
   int tag;
   PACKET_new(p);

   /* Receive the control stream */
   int rc = NETWORK_recv(net, &tag, p, &stControl, true);
   PACKET_delete(p);
   if ( rc != 1 )
   {
      cerr << "[BE " << WhoAmI() << "] net::recv() failure" << endl;
      return -1;
   }
   return 0;
}


/**
 * Connects the backend to the network (remote instantiation) reading the connection information from a file.
 * @param wRank           Backend world rank.
 * @param connectionsFile Backends connections file.
 * @return The MRNet Network on success; NULL otherwise.
 */
NETWORK * BackEnd::Connect(int wRank, const char *connectionsFile)
{
   /* Get connection information */
   char parHostname[64], parPort[10], parRank[10];

   PendingConnections BE_connex(connectionsFile);
   if (BE_connex.GetParentInfo(wRank, parHostname, parPort, parRank) == -1) 
   {
      cerr << "[BE " << wRank << "] failed to parse connections file '" << connectionsFile << "'." << endl;
      return NULL;
   }

   /* Connect the BE to the network */
   return Connect(wRank, (char *)parHostname, (char *)parPort, (char *)parRank);
}

/*
 * Connects the backend to the network (remote instantiation) passing the connection information by parameter.
 * If your back-ends use this method directly, you will have to externally distribute the connection information, 
 * for example, through MPI. This is necessary at large-scale, because all back-ends parsing the same file 
 * simultaneously collapses the file system.
 * @param wRank           Back-end world rank. 
 * @param myHostname      Back-end's hostname.
 * @param parHostname     The parent hostname.
 * @param parPort         The parent port.
 * @param parRank         The parent rank.
 * @return The MRNet Network on success; NULL otherwise.
 */
NETWORK * BackEnd::Connect(int wRank, char *parHostname, char *parPort, char *parRank)
{
   char myHostname[64], myRank[10];
   while( gethostname(myHostname, 64) == -1 ) {}
   myHostname[63] = '\0';

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
void BackEnd::Loop(callback_function preProtocol, callback_function postProtocol)
{
   Protocol *prot;
   int next_tag;
   int err = 0;
   char *prot_id;
   PACKET_new(p);

   if (stControl == NULL) return;
   
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
            if (preProtocol  != NULL) preProtocol(prot_id, prot);
            err = prot->Run();
            if (postProtocol != NULL) postProtocol(prot_id, prot);
         }
         /* Notify success or errors  */
         MRN_STREAM_SEND(stControl, TAG_ACK, "%d", err*(-1)); /* 0 success, +1 error */
      } 
   } while (next_tag != TAG_EXIT);

   PACKET_delete(p);
   Shutdown();
}

void BackEnd::Loop()
{
  Loop(NULL, NULL);
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

   // NETWORK_delete(net); /* Freeing the network in the BE's crashes due to double-free! */

   sleep(3);
}


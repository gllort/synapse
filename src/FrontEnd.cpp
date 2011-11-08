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
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "FrontEnd.h"
#include "FrontProtocol.h"

using namespace std;
using namespace MRN;


/**
 * FrontEnd constructor 
 */
FrontEnd::FrontEnd()
{
   numBackendsConnected = 0;
}


/**
 * Called from the superclass to identify which type of MRNet process this is.
 * @return true. 
 */
bool FrontEnd::isFE()
{
   return true;
}


/** 
 * MRNet callback that is invoked when a backend connects to the network.
 * @param evt The event that triggers this callback.
 */
void BE_Join_Callback(Event *evt, void *fe_ptr)
{
   FrontEnd * fe = (FrontEnd *)fe_ptr;
   if ( (evt->get_Class() == Event::TOPOLOGY_EVENT) &&
        (evt->get_Type() == TopologyEvent::TOPOL_ADD_BE) )
   {
      fe->numBackendsConnected ++;
   }
}


/**
 * MRNet callback that is invoked when a backend fails or disconnects from the network.
 * @param evt The event that triggers this callback.
 */
void BE_Quit_Callback(Event *evt, void *fe_ptr)
{
   FrontEnd * fe = (FrontEnd *)fe_ptr;
   /* XXX Cannot differentiate whether the backend quits or fails */
   if ( (evt->get_Class() == Event::TOPOLOGY_EVENT) &&
        (evt->get_Type() == TopologyEvent::TOPOL_REMOVE_NODE) )
   {
      fe->numBackendsConnected --;
   }
}


/**
 * Returns the number of back-ends that are connected to the network.
 * @return Number of back-ends connected.
 */
int FrontEnd::ConnectedBackEnds()
{
   return numBackendsConnected;
}


/** 
 * Instantiates the MRNet and spawns the backends.
 * @param TopologyFile The topology of the network (including backends).
 * @param BackendExe The backend executable to start.
 * @param BackendArgs The arguments of the backend.
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int FrontEnd::Init(const char *TopologyFile, const char *BackendExe, const char **BackendArgs)
{
   bool cbrett = false;

   /* Start MRNet and backends */
   net = Network::CreateNetworkFE( TopologyFile, BackendExe, BackendArgs );
   if ( net->has_Error() )
   {
      net->perror("[FE] ERROR: Network creation failed");
      return -1;
   }

   /* Register callbacks */
   if ( ! net->set_FailureRecovery(false) )
   {
      cerr << "[FE] ERROR: Failed to disable failure recovery" << endl;
      delete net;
      return -1;
   }

   cbrett = net->register_EventCallback( Event::TOPOLOGY_EVENT,
                                         TopologyEvent::TOPOL_REMOVE_NODE,
                                         BE_Quit_Callback, (void *)this );
   if( cbrett == false )
   {
      cerr << "[FE] ERROR: Failed to register callback for backend disconnection" << endl;
      delete net;
      return -1;
   }

   return Init();
}


/** 
 * Normal instantiation that reads the topology from the environment variable MRNAPP_TOPOLOGY. 
 * @param BackendExe The backend executable to start.
 * @param BackendArgs The arguments of the backend.
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int FrontEnd::Init(const char *BackendExe, const char **BackendArgs)
{
   char *env_MRNAPP_TOPOLOGY = getenv("MRNAPP_TOPOLOGY");
   if (env_MRNAPP_TOPOLOGY == NULL)
   {
      cerr << "[FE] ERROR: MRNAPP_TOPOLOGY environment variable is not defined!" << endl; 
      cerr << "[FE] Make it point to the MRNet topology file." << endl;
      return -1;
   }
   return Init(((const char *)env_MRNAPP_TOPOLOGY), BackendExe, BackendArgs);
}


/**
 * Instantiates the MRNet except the backends, and waits for these to connect.
 * @param TopologyFile    Topology of the network (not including backends).
 * @param numBackends     Number of backends that will be manually spawned.
 * @param ConnectionsFile File where backends connections will be written to.
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int FrontEnd::Init(const char *TopologyFile, unsigned int numBackends, const char *ConnectionsFile) 
{
   bool cbrett = false;

   No_BE_Instantiation = true;

   /* Start MRNet without the backends */
   net = Network::CreateNetworkFE( TopologyFile, NULL, NULL );
   if ( net->has_Error() )
   {
      net->perror("[FE] ERROR: Network creation failed");
      return -1;
   }

   /* Register callbacks */
   if ( ! net->set_FailureRecovery(false) )
   {
      cerr << "[FE] ERROR: Failed to disable failure recovery" << endl;
      delete net;
      return -1;
   }

   cbrett = net->register_EventCallback( Event::TOPOLOGY_EVENT,
                                         TopologyEvent::TOPOL_ADD_BE,
                                         BE_Join_Callback, (void *)this );
   if (cbrett == false) 
   {
      cerr << "[FE] ERROR: Failed to register callback for backend connection" << endl;
      delete net;
      return -1;
   }

   cbrett = net->register_EventCallback( Event::TOPOLOGY_EVENT,
                                         TopologyEvent::TOPOL_REMOVE_NODE,
                                         BE_Quit_Callback, (void *)this );
   if( cbrett == false )
   {
      cerr << "[FE] ERROR: Failed to register callback for backend disconnection" << endl;
      delete net;
      return -1;
   }

   if ( WaitForBackends(numBackends, ConnectionsFile) != 0 ) 
   {
      delete net;
      return -1;
   }

   return Init();
}


/**
 * No back-ends instantiation that reads the topology from the environment variable MRNAPP_TOPOLOGY.
 * @param numBackends     Number of backends that will be manually spawned.
 * @param ConnectionsFile File where backends connections will be written to.
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int FrontEnd::Init(unsigned int numBackends, const char *ConnectionsFile)
{
   char *env_MRNAPP_TOPOLOGY = getenv("MRNAPP_TOPOLOGY");
   if (env_MRNAPP_TOPOLOGY == NULL)
   {
      cerr << "[FE] ERROR: MRNAPP_TOPOLOGY environment variable is not defined!" << endl;
      cerr << "[FE] Make it point to the MRNet topology file." << endl;
      return -1;
   }
   return Init(((const char *)env_MRNAPP_TOPOLOGY), numBackends, ConnectionsFile);
}


/**
 * Common initialization creates the control stream and sends it to the backends.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::Init()
{
   /* A broadcast communicator contains all the back-ends */
   Communicator *comm_BC     = net->get_BroadcastCommunicator( );
 
   /* Create and announce control stream */
   stControl = net->new_Stream( comm_BC, TFILTER_SUM, SFILTER_WAITFORALL );

   if (( stControl->send( TAG_STREAM, "" ) == -1 ) || ( stControl->flush() == -1 )) 
   {
      cerr << "[FE] stControl::send() failure" << endl;
      delete stControl;
      delete net;
      return -1;
   }
   return 0;
}


/**
 * Waits for all the backends to connect to the network. 
 * @param numBackends     Number of backends to wait for. 
 * @param ConnectionsFile File where backends connections will be written to.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::WaitForBackends(unsigned int numBackends, const char *ConnectionsFile)
{
   /* Query Network for topology object */
   NetworkTopology *netTopology = net->get_NetworkTopology();
   vector< NetworkTopology::Node * > internalLeaves;
   netTopology->get_Leaves(internalLeaves);
   netTopology->print(stdout);

   /* Write connection information to the specified file */
   if (WriteConnections(internalLeaves, numBackends, ConnectionsFile) != 0)
   {
      cerr << "[FE] ERROR: Cannot write connections file '" << ConnectionsFile << "'" << endl;
      return -1;
   }

   /* Wait for backends to attach */
   unsigned int retries=0;
   unsigned int countWaitFor=numBackends, lastWaitFor=0;
   do
   {
      countWaitFor = numBackends - numBackendsConnected;
      if (countWaitFor != lastWaitFor)
         cout << "Waiting for " << countWaitFor << " backends to connect..." << endl;
      lastWaitFor = countWaitFor;
      retries ++;
      sleep(1);
   } while ((numBackendsConnected != numBackends) && (retries < MAX_RETRIES));

   if (numBackendsConnected != numBackends)
   {
      cerr << "[FE] ERROR: Connection time-out! " 
           << numBackends - numBackendsConnected 
           << " backends failed to connect within " 
           << MAX_RETRIES << " seconds" << endl;
      return -1;
   }
   else
   {
      cout << "[FE] " << numBackends << " backends connected!" << endl;
      return 0;
   }
}


/**
 * Writes the backends connections to the specified file.
 * @param internalLeaves  Leaves of the MRNet where the backends will connect to.
 * @param numBackends     Number of backends that have to connect.
 * @param ConnectionsFile File where the backends connections will be written to.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::WriteConnections(
   vector< NetworkTopology::Node * >& internalLeaves, 
   unsigned int                       numBackends, 
   const char                        *ConnectionsFile)
{
   FILE *f;
   if ( (f = fopen(ConnectionsFile, (const char *)"w+")) == NULL ) 
   {
      perror("fopen");
      return -1;
   }
     
   unsigned num_leaves  = internalLeaves.size();
   unsigned be_per_leaf = numBackends / num_leaves;
   unsigned curr_leaf   = 0;
   for(unsigned i=0; (i < numBackends) && (curr_leaf < num_leaves); i++) 
   {
       if( i && (i % be_per_leaf == 0) ) 
       {
           // select next parent
           curr_leaf++;
           if( curr_leaf == num_leaves ) 
           {
               // except when there is no "next"
               curr_leaf--;
           }
       }
       cout << "BE " << i << " will connect to " 
            << internalLeaves[curr_leaf]->get_HostName().c_str() << ":" 
            << internalLeaves[curr_leaf]->get_Port()             << ":"
            << internalLeaves[curr_leaf]->get_Rank()             << endl;

       fprintf(f, "%s %d %d %d\n",
               internalLeaves[curr_leaf]->get_HostName().c_str(),
               internalLeaves[curr_leaf]->get_Port(),
               internalLeaves[curr_leaf]->get_Rank(),
               i);
   }
   fclose(f);
   return 0;
}


/**
 * Tells the back-ends the next protocol we are going to execute and runs it.
 * @param prot_id The protocol identifier.
 * @param prot    Protocol object is returned by reference to retrieve results.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::Dispatch(string prot_id, Protocol *& prot)
{
   int rc = 0;

   /* Get the protocol object */
   prot = MRNetApp::FetchProtocol(prot_id);

   if (prot != NULL)
   {
      int tag, countErr = 0;
      PacketPtr(p);

      cout << "[FE] Dispatching " << prot_id << endl;
      /* Announce the next protocol to execute to the back-ends */
      MRN_STREAM_SEND(stControl, TAG_PROT_ID, "%s", prot_id.c_str());

      /* Run the front-end side of the protocol */
      rc = prot->Run();

      /* Receive ACKs from the back-ends */
      MRN_STREAM_RECV(stControl, &tag, p, TAG_ACK);
      p->unpack("%d", &countErr);
      if (countErr != 0)
      {
         /* Some BEs had errors! */
         cerr << "[FE] " << prot_id << ": ERROR: " << countErr << " back-ends failed!" << endl; 
         return -1;
      } 
   }
   else
   {
      /* Protocol prot_id is not loaded in the front-end! */
      cerr << "[FE] Error: Protocol '" << prot_id << "' is not loaded!" << endl;
      return -1;
   }
   cout << "[FE] " << prot_id << ": SUCCESS" << endl; 
   return 0;
}


/**
 * Tokenizes the string s by the delimiter delim.
 * @param s Input string.
 * @param delim Separator character.
 * @param tokens Output array with the resulting tokens.
 * @return The size of the array.
 */
int split(const std::string &s, char delim, std::vector<std::string> &tokens)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens.size();
}


/**
 * Looks for the filter shared object specified by filter_name (appending .so) 
 * in the paths specified with the environment variable MRNAPP_FILTER_PATH. If the
 * filter is found, it is loaded into the network.
 * @param filter_name Name of the filter shared object.
 * @return the filter id; or -1 if can not be found or loaded. 
 */
int FrontEnd::LoadFilter(string filter_name)
{
   if (filter_name != "")
   {
      string paths(".");
      char  *env_filter_path = getenv("MRNAPP_FILTER_PATH");

      if (env_filter_path != NULL) paths += ":" + string(env_filter_path);

      vector<string> paths_array;
      split(paths, ':', paths_array);

      for (unsigned int i=0; i<paths_array.size(); i++)
      {
         string filter_so(""), filter_func("filter");

         if (paths_array[i].size() > 0)
         {
            filter_so += paths_array[i];
            if (paths_array[i][paths_array[i].size()-1] != '/') filter_so += "/";
         }
         filter_so += "libfilter";
         filter_so += filter_name;
         filter_so += ".so";

         filter_func += filter_name;

         ifstream fd(filter_so.c_str());
         if (fd.good())
         {
            int filter_id = net->load_FilterFunc( filter_so.c_str(), filter_func.c_str() );
            if (filter_id == -1)
            {
               cerr << "[FE] Error loading filter " << filter_so << endl;
               return -1;
            }
            else
            {
               cout << "[FE] Filter " << filter_so << " (routine=" << filter_func << ", ID=" << filter_id << ") loaded successfully!" << endl;
               return filter_id;
            }
         }
      }
      cerr << "[FE] Filter '" << filter_name << "' not found!" << endl;
      return -1;
   }
   return -1;
}


/**
 * Loads an user-protocol for the front-end side of the MRNet.
 * @param prot Protocol to load in the front-end.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::LoadProtocol(Protocol *prot)
{
	((FrontProtocol *)prot)->Init(this);
    return MRNetApp::LoadProtocol(prot);
}


/**
 * Notifies the back-ends to exit and shutdowns the MRNet.
 * @return 0 on success; -1 otherwise.
 */
void FrontEnd::Shutdown()
{
   int tag;
   PacketPtr p;

   /* Tell back-ends to exit */
   MRN_STREAM_SEND(stControl, TAG_EXIT, "");
   /* Wait for ACKs */
   MRN_STREAM_RECV(stControl, &tag, p, TAG_EXIT);

   /* Back-ends are waiting on stControl to be closed */
   delete stControl;

   cout << "[FE] Exiting!" << endl;

   /* The Network destructor will cause all internal and leaf tree nodes to exit */
   delete net;
   sleep(3);
}


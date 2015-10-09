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
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "FrontEnd.h"
#include "FrontProtocol.h"
#include "PendingConnections.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using namespace MRN;
using namespace Synapse;


/**
 * FrontEnd constructor 
 */
FrontEnd::FrontEnd()
{
   numBackendsConnected   = 0;
   ConnectionsFileWritten = false;
   PendingBackends        = 0;
   InitCompleted          = false;
   ShutdownCalled         = false; 
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

   return CommonInit();
}


/** 
 * Normal instantiation that reads the topology from the environment variable SYNAPSE_TOPOLOGY. 
 * @param BackendExe The backend executable to start.
 * @param BackendArgs The arguments of the backend.
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int FrontEnd::Init(const char *BackendExe, const char **BackendArgs)
{
   char *env_SYNAPSE_TOPOLOGY = getenv("SYNAPSE_TOPOLOGY");
   if (env_SYNAPSE_TOPOLOGY == NULL)
   {
      cerr << "[FE] ERROR: SYNAPSE_TOPOLOGY environment variable is not defined!" << endl; 
      cerr << "[FE] Make it point to the MRNet topology file." << endl;
      return -1;
   }
   return Init(((const char *)env_SYNAPSE_TOPOLOGY), BackendExe, BackendArgs);
}


/**
 * Instantiates the MRNet except the backends, and waits for these to connect.
 * @param TopologyFile    Topology of the network (not including backends).
 * @param numBackends     Number of backends that will be manually spawned.
 * @param ConnectionsFile File where backends connections will be written to.
 * @param wait_for_BEs    Optional argument set to true by default. In this case,
 *                        the front-end waits for the back-ends to connect and 
 *                        completes the initialization. Otherwise, the user will
 *                        call to Connect() later to complete the initialization.
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int FrontEnd::Init(const char *TopologyFile, unsigned int numBackends, const char *ConnectionsFile, bool wait_for_BEs) 
{
   bool cbrett = false;

   Remote_Instantiation = true;

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

   /* Write connection information to the specified file */
   PendingConnections BE_connex(ConnectionsFile);
   if (BE_connex.Write(net, numBackends) != 0)
   {
      cerr << "[FE] ERROR: Cannot write connections file '" << ConnectionsFile << "'" << endl;
      delete net;
      return -1;
   }
   else ConnectionsFileWritten = true;

   /* Do the last part of the initialization right away or return the control to the user 
    * who will call Connect() manually. 
    */
   PendingBackends = numBackends;
   if (wait_for_BEs)
   {
     return Connect();
   }
   else
   {
     return 0;
   }
}

/**
 * No back-ends instantiation that reads the topology from the environment variable SYNAPSE_TOPOLOGY,
 * the number of back-ends from SYNAPSE_NUM_BE, and the connections file from SYNAPSE_BE_CONNECTIONS.
 *
 * @param wait_for_BEs    Optional argument set to true by default. In this case,
 *                        the front-end waits for the back-ends to connect and 
 *                        completes the initialization. Otherwise, the user will
 *                        call to Connect() later to complete the initialization.
 *
 * @return 0 if the MRNet starts successfully; -1 otherwise.
 */
int FrontEnd::Init(bool wait_for_BEs)
{
   char *env_SYNAPSE_TOPOLOGY = getenv("SYNAPSE_TOPOLOGY");
   if (env_SYNAPSE_TOPOLOGY == NULL)
   {
      cerr << "[FE] ERROR: SYNAPSE_TOPOLOGY environment variable is not defined!" << endl;
      cerr << "[FE] Make it point to the MRNet topology file." << endl;
      return -1;
   }
   char *env_SYNAPSE_NUM_BE = getenv("SYNAPSE_NUM_BE");
   if (env_SYNAPSE_NUM_BE == NULL)
   {
      cerr << "[FE] ERROR: SYNAPSE_NUM_BE environment variable is not defined!" << endl;
      cerr << "[FE] Specify how many back-ends are you going to launch." << endl;
      return -1;
   }
   char *env_SYNAPSE_BE_CONNECTIONS = getenv("SYNAPSE_BE_CONNECTIONS");
   if (env_SYNAPSE_BE_CONNECTIONS == NULL)
   {
      cerr << "[FE] ERROR: SYNAPSE_BE_CONNECTIONS environment variable is not defined!" << endl;
      cerr << "[FE] Make it point to the back-ends connection file." << endl;
      return -1;
   }
   return Init(((const char *)env_SYNAPSE_TOPOLOGY), atoi(env_SYNAPSE_NUM_BE), ((const char *)env_SYNAPSE_BE_CONNECTIONS), wait_for_BEs);
}


/**
 * In the remote instantiation mode, this is the second part of the Init() function. 
 * Init() can call this function automatically if specified, otherwise you have to 
 * call it manually. This was implemented to allow the use-case where front-end and
 * back-ends are threads of an MPI program and you need to get the control back after
 * the initialization to distribute the pending connection information among the 
 * MPI tasks and then start the back-ends. 
 *
 * @return 0 on success; -1 otherwise;
 */
int FrontEnd::Connect()
{
   /* Wait for back-ends to connect */
   if ( WaitForBackends(PendingBackends) != 0 )  
   {
      delete net;
      return -1;
   }
   return CommonInit();
}


/**
 * Common initialization creates the control stream and sends it to the backends.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::CommonInit()
{
   /* A broadcast communicator contains all the back-ends */
   Communicator *comm_BC     = net->get_BroadcastCommunicator( );
 
   /* Create and announce control stream */
#if defined(CONTROL_STREAM_BLOCKING)
   stControl = net->new_Stream( comm_BC, TFILTER_SUM, SFILTER_WAITFORALL );
#else
   stControl = net->new_Stream( comm_BC, TFILTER_NULL, SFILTER_DONTWAIT );
#endif

   if (( stControl->send( TAG_STREAM, "" ) == -1 ) || ( stControl->flush() == -1 )) 
   {
      cerr << "[FE] stControl::send() failure" << endl;
      delete stControl;
      delete net;
      return -1;
   }

   InitCompleted = true;
   return 0;
}


/**
 * Waits for all the backends to connect to the network. 
 * @param numBackends     Number of backends to wait for. 
 * @param ConnectionsFile File where backends connections will be written to.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::WaitForBackends(unsigned int numBackends) 
{
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
   } while ((numBackendsConnected != numBackends) && (retries < MAX_WAIT_RETRIES));

   if (numBackendsConnected != numBackends)
   {
      cerr << "[FE] ERROR: Connection time-out! " 
           << numBackends - numBackendsConnected 
           << " backends failed to connect within " 
           << MAX_WAIT_RETRIES << " seconds" << endl;
      return -1;
   }
   else
   {
      cout << "[FE] " << numBackends << " backends connected!" << endl;
      return 0;
   }
}

bool FrontEnd::isUp()
{
  return (InitCompleted) && (!ShutdownCalled);
}


/**
 * Returns true if the connections file with the back-ends attachment information for the
 * no-BE instantiation method has already been written.
 * @return true if the file is written; false otherwise.
 */
bool FrontEnd::isConnectionsFileWritten()
{
   return ConnectionsFileWritten;
}

/**
 * Tells the back-ends the next protocol we are going to execute and runs it.
 * @param prot_id The protocol identifier.
 * @param status  Set to the return code of the protocol that is run.
 * @param prot    Protocol object is returned by reference to retrieve results.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::Dispatch(string prot_id, int &status, Protocol *& prot)
{
   status = -1;

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
      status = prot->Run();

      /* Receive ACKs from the back-ends */
#if defined(CONTROL_STREAM_BLOCKING)
      MRN_STREAM_RECV(stControl, &tag, p, TAG_ACK);
      p->unpack("%d", &countErr);
#else
      for (int i=0; i<stControl->size(); i++)
      {
        int x = 0;
        MRN_STREAM_RECV(stControl, &tag, p, TAG_ACK);
        p->unpack("%d", &x);
        countErr += x;
      }
#endif
      /* DEBUG 
      std::cout << "FrontEnd::Dispatch: Received ACK's countErr=" << countErr << std::endl; */
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
 * Wrapper for Dispatch(string, Protocol *&) that does not return the protocol by reference.
 * @param prot_id The protocol identifier.
 * @param status  Set to the return code of the protocol that is run.
 * @return 0 on success; -1 otherwise.
 */
int FrontEnd::Dispatch(string prot_id, int &status)
{
   Protocol *prot = NULL;
   return Dispatch(prot_id, status, prot);
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
 * in the paths specified with the environment variable SYNAPSE_FILTER_PATH. If the
 * filter is found, it is loaded into the network.
 * @param filter_name Name of the filter shared object.
 * @return the filter id; or -1 if can not be found or loaded. 
 */
int FrontEnd::LoadFilter(string filter_name)
{
   if (filter_name != "")
   {
      string paths(".");
      char  *env_filter_path = getenv("SYNAPSE_FILTER_PATH");

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
               cerr << "[FE] Error loading filter " << filter_so << ": Function '" << filter_func.c_str() << "' is present in the filter object?" << endl;
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

   ShutdownCalled = true;

   if (InitCompleted) 
   {
     /* Tell back-ends to exit */
     MRN_STREAM_SEND(stControl, TAG_EXIT, "");

     /* Wait for ACKs */
#if defined(CONTROL_STREAM_BLOCKING)
     MRN_STREAM_RECV(stControl, &tag, p, TAG_EXIT);
#else
     for (int i=0; i<stControl->size(); i++)
     {
       MRN_STREAM_RECV(stControl, &tag, p, TAG_EXIT);
     }
#endif
     /* Back-ends are waiting on stControl to be closed */
     delete stControl;
   }

   cout << "[FE] Exiting!" << endl;

   /* The Network destructor will cause all internal and leaf tree nodes to exit */
   delete net;
   sleep(3);
}


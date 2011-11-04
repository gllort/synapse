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
#include <string>
#include <vector>
#include "MRNetApp.h"
#include "Protocol.h"


/**
 * Generic FrontEnd/BackEnd constructor.
 */
MRNetApp::MRNetApp()
{
   net       = NULL;
   stControl = NULL;
   No_BE_Instantiation = false;
}


/**
 * Returns the MRNet network object.
 * @return the network.
 */
NETWORK * MRNetApp::GetNetwork()
{
   return net;
}


/**
 * Returns the MRNet control stream object.
 * @return the control stream.
 */
STREAM * MRNetApp::GetControlStream()
{
   return stControl;
}


/**
 * Returns the rank of the current MRNet process.
 * @return the MRNet process rank.
 */
unsigned int MRNetApp::WhoAmI(void)
{
   unsigned int mrnID = 0;
   if (net != NULL)
   {
      mrnID = NETWORK_get_LocalRank(net);
   }
   else 
   {
      cerr << "ERROR: MRNetApp::WhoAmI() called but Network is not yet initialized!" << endl;
   }
   return ((isBE() && No_BE_Instantiation) ? MPI_RANK(mrnID) : mrnID);
}


/**
 * Checks the Network Topology for the number of back-ends.
 * @return number of back-ends in the MRNet.
 */
unsigned int MRNetApp::NumBackEnds(void)
{
   unsigned int num_be;
   NETWORK_get_NumBackEnds(net, num_be);
   return num_be;
}

/**
 * Keeps a mapping of the loaded protocols, indexed by the protocol ID.
 * @param prot The protocol that is being loaded.
 * @return 0 on success; -1 otherwise.
 */
int MRNetApp::LoadProtocol(Protocol *prot)
{
   if (prot->ID() != "")
   {
      loadedProtocols[prot->ID()] = prot;
   }
   return 0;
}


/**
 * Returns the loaded protocol object identified by ID.
 * @param ID Protocol identifier.
 * @return the protocol object.
 */
Protocol * MRNetApp::FetchProtocol(string ID)
{
   map<string, Protocol*>::iterator it;

   it = loadedProtocols.find(ID);
   if (it != loadedProtocols.end())
   {
      return it->second;
   }
   else
   {
      return NULL;
   }
}


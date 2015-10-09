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

#include "Protocol.h"
#include "MRNetApp.h"

using namespace Synapse;

Protocol::Protocol()
{
   mrnApp = NULL;
}

/**
 * Returns the back-end identifier.
 * @return the MRNetID of the back-end that has this protocol loaded.
 */
unsigned int Protocol::WhoAmI(bool return_network_id)
{
   return mrnApp->WhoAmI(return_network_id);
}


/**
 * Returns the number of back-ends in the network where this protocol is loaded.
 * @return number of back-ends.
 */
unsigned int Protocol::NumBackEnds()
{
   return mrnApp->NumBackEnds();
}


/**
 * Returns the MRNet Network object where this protocol is loaded.
 * @return the MRNet Network.
 */
NETWORK * Protocol::GetNetwork()
{
   return mrnApp->GetNetwork();
}


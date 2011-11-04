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

#ifndef __MRNET_APP_H__
#define __MRNET_APP_H__

#include <map>
#include <string>
#include "MRNet_wrappers.h"

#define MRNET_RANK(r) (r+1000000)
#define MPI_RANK(r)   (r-1000000)

using std::map;
using std::string;

class Protocol;

class MRNetApp 
{
   public:
      NETWORK * net;
      STREAM  * stControl;

      MRNetApp();
      virtual ~MRNetApp() { };

      NETWORK * GetNetwork       (void);
      STREAM  * GetControlStream (void);
      int       LoadProtocol     (Protocol *prot);
      Protocol* FetchProtocol    (string prot_id);
      unsigned int NumBackEnds   (void);
      unsigned int WhoAmI        (void);
      virtual bool isFE (void) { return false; };
      virtual bool isBE (void) { return false; };

   protected:
      bool No_BE_Instantiation; /* Network instantiation mode; 
                                   true=no back-ends, false=normal */

   private:
      map<string, Protocol*> loadedProtocols; /* Mapping of user-defined protocols that are loaded 
                                                 into the FE/BE, indexed by their ID */
};

#endif /* __MRNET_APP_H__ */

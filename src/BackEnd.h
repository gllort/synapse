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

#ifndef __BACKEND_H__
#define __BACKEND_H__

#include "MRNetApp.h"

#define MRNET_RANK(r) (r+1000000)
#define MPI_RANK(r)   (r-1000000)
#define WHOAMI        (No_BE_Instantiation ? MPI_RANK(WhoAmI) : WhoAmI)

class BackEnd : public MRNetApp
{
   public:
      BackEnd();

      int  Init(int argc,  char *argv[]);
      int  Init(int wRank, const char *connectionsFile);
      int  LoadProtocol(Protocol *prot);

      void Loop();
      void Shutdown();

   private:
      unsigned int WhoAmI;

      int       Init();
      NETWORK * Connect(int wRank, const char *connectionsFile);
      int       getParentInfo(const char *file, int rank, char *phost, char *pport, char *prank);
};

#endif /* __BACKEND_H__ */

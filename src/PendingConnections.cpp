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

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include "PendingConnections.h"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::ifstream;
using namespace Synapse;

PendingConnections::PendingConnections(string ConnectionsFile)
{
  this->ConnectionsFile = ConnectionsFile;
}

/**
 * Writes the pending connections to the specified file.
 * @param net             The MRNet network object.
 * @param numBackends     Number of backends that have to connect.
 * @return 0 on success; -1 otherwise.
 */
int PendingConnections::Write(
  NETWORK     *net,
  unsigned int numBackends)
{
  /* Query Network for topology object */
  NetworkTopology *netTopology = net->get_NetworkTopology();
  vector< NetworkTopology::Node * > internalLeaves;
  netTopology->get_Leaves(internalLeaves);
  netTopology->print(stdout);

  FILE *f;
  if ( (f = fopen(ConnectionsFile.c_str(), (const char *)"w+")) == NULL )
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
 * Retrieves host, port and rank where a given backend connects from the connections file.
 * @param rank Backend rank.
 * @return Host, port and rank where this backend will connect and 0 on success; -1 otherwise.
 */
int PendingConnections::GetParentInfo( int rank, char *phost, char *pport, char *prank )
{
  int keep_retrying = MAX_PARSE_RETRIES;

  while (keep_retrying > 0)
  {
    ifstream ifs(ConnectionsFile.c_str());
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
          cerr << "PendingConnections::GetParentInfo: ERROR: scanning '" << ConnectionsFile << "'\n";
        }
        else if( trank == rank )
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
    cerr << "PendingConnections::GetParentInfo: ERROR: retrieving parent information for back-end " << rank << "." << endl;
    cerr << "Did the front-end finish writing the connections file? Retrying in " << IDLE_BETWEEN_TRIES << " second(s)..." << endl;
    sleep(IDLE_BETWEEN_TRIES);
    keep_retrying --;
  }
  // my rank not found :(
  return -1;
}

/**
 * Parses the connections file and serializes the data into arrays to be distributed through MPI scatter.
 * This method is meant for the use-case where the back-ends are spawned through MPI, and just one 
 * process reads the connection information and distributes the data to the rest through MPI. This 
 * was implemented to avoid stressing the filesystem because of many back-ends reading the same 
 * file simultaneously.
 * @param world_size The total number of back-ends.
 * @param sendbuf    Address of send buffer to store the data to send to each process.
 * @param sendcnt    Address of integer array to specify in entry i the number of elements to send to processor i.
 * @param displs     Adress of integer to specify in entry i the displacement (relative to sendbuf) from which to take the outgoing data to process i.
 */
int PendingConnections::ParseForMPIDistribution(int world_size, char *&sendbuf, int *&sendcnts, int *&displs)
{
  int keep_retrying = MAX_PARSE_RETRIES;

  while(keep_retrying > 0)
  {
    int rbytes = 0;
    int offset = 0, ntasks = 0;
    string line;
    ifstream fd;

    fd.open(ConnectionsFile.c_str());

    if (fd == NULL)
    {
      cerr << "PendingConnections::ParseForMPIDistribution: ERROR: opening connections file '" << ConnectionsFile << " '" << endl;
    }
    else
    {
      sendcnts = (int *)malloc(world_size * sizeof(int));
      bzero(sendcnts, world_size * sizeof(int));
      displs   = (int *)malloc(world_size * sizeof(int));
      bzero(displs, world_size * sizeof(int));

      /* Read connections file line by line */
      while (getline(fd, line))
      {
        rbytes = line.size();

        /* Store the lines in a memory-consecutive buffer */
        sendbuf = (char *)realloc(sendbuf, offset+rbytes+1);
        strcpy(&(sendbuf[offset]), line.c_str());

        /* Store the length and offset of every line */
        sendcnts[ntasks] = rbytes + 1; /* strlen(line) + '\0' */
        displs[ntasks]   = offset;

        /* DEBUG
        fprintf(stderr, "sendcnts=%d displs=%d data=%s", 
          sendcnts[ntasks], 
          displs[ntasks], 
          &sendbuf[offset]); */

        offset += rbytes+1;
        ntasks ++;
      }
      fd.close();

      /* Check there's the same number of MPI tasks than BE's */
      if (ntasks == world_size) 
      {
        /* All data was parsed ok, exit normally */
        return 0;
      }
      else
      {
        cerr << "PendingConnections::ParseForMPIDistribution: ERROR: unexpected number of tasks in connections file '"
             << ConnectionsFile << "' (found " << ntasks << " task(s), " << world_size << " expected)" << endl;
        free(sendbuf);
        free(sendcnts);
        free(displs);
        sendbuf  = NULL;
        sendcnts = NULL;
        displs   = NULL;
      }
    }
    cerr << "Did the front-end finish writing the connections file? Retrying in " << IDLE_BETWEEN_TRIES << " second(s)..." << endl;
    sleep(IDLE_BETWEEN_TRIES);
    keep_retrying --;
  }
  return -1;
}


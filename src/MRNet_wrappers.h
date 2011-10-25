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

#ifndef __MRNET_WRAPPERS_H__
#define __MRNET_WRAPPERS_H__

#include <stdlib.h>
#include "MRNet_tags.h"


/**
 * The following macros wrap many of the MRNet types and some API calls so that we
 * can use the normal or the lightweight libraries indistinctly.
 */
#if defined(LIGHTWEIGHT)
# ifdef __cplusplus
extern "C" {
# endif
#  include <mrnet_lightweight/MRNet.h>
# ifdef __cplusplus
}
# endif
# define STREAM_recv(stream, tag, data, block)       Stream_recv(stream, tag, data, block)
# define STREAM_get_Id(stream)                       Stream_get_Id(stream)
# define STREAM_send(stream, tag, format, args...)   Stream_send(stream, tag, format, ## args)
# define STREAM_flush(stream)                        Stream_flush(stream)
# define STREAM                                      Stream_t
# define STREAM_PTR                                  Stream_t*
# define STREAM_is_Closed(stream)                    Stream_is_Closed(stream)
# define PACKET                                      Packet_t
# define PACKET_PTR                                  Packet_t*
# define PACKET_new(p)                               PACKET_PTR p = (PACKET_PTR)malloc(sizeof(PACKET))
# define PACKET_unpack(p, fmt, args...)               Packet_unpack(p, fmt, ## args)
# define PACKET_delete(p)                            if (p != NULL) free(p)
# define NETWORK                                     Network_t
# define NETWORK_PTR                                 Network_t*
# define NETWORK_recv(net, tag, data, stream, block) ( block ? Network_recv(net, tag, data, stream) : Network_recv_nonblock(net, tag, data, stream) )
# define NETWORK_CreateNetworkBE(argc, argv)         Network_CreateNetworkBE(argc, argv)
# define NETWORK_get_LocalRank(net)                  Network_get_LocalRank(net)
# define NETWORK_waitfor_ShutDown(net)               Network_waitfor_ShutDown(net)
# define NETWORK_delete(net)                         if (net != NULL) delete_Network_t(net)
#else
# include <mrnet/MRNet.h>
using namespace MRN;
# define STREAM_recv(stream, tag, data, block)       stream->recv(tag, data, block)
# define STREAM_get_Id(stream)                       stream->get_Id()
# define STREAM_send(stream, tag, format, args...)   stream->send(tag, format, ## args)
# define STREAM_flush(stream)                        stream->flush()
# define STREAM                                      Stream
# define STREAM_PTR                                  Stream*
# define STREAM_is_Closed(stream)                    stream->is_Closed()
# define PACKET                                      Packet
# define PACKET_PTR                                  PacketPtr
# define PACKET_new(p)                               PACKET_PTR p
# define PACKET_unpack(p, fmt, args...)              p->unpack(fmt, ## args)
# define PACKET_delete(p)                              
# define NETWORK                                     Network
# define NETWORK_PTR                                 Network*
# define NETWORK_recv(net, tag, data, stream, block) net->recv(tag, data, stream, block)
# define NETWORK_CreateNetworkBE(argc, argv)         Network::CreateNetworkBE(argc, argv)
# define NETWORK_get_LocalRank(net)                  net->get_LocalRank()
# define NETWORK_waitfor_ShutDown(net)               net->waitfor_ShutDown()
# define NETWORK_delete(net)                         if (net != NULL) delete net
/* Sends a message to the subset of BEs in the stream specified in be_list */
# define MRN_STREAM_SEND_P2P(stream, be_list, tag, format, args...)    \
{                                                                      \
	PacketPtr p( new Packet(stream->get_Id(), tag, format, ## args) ); \
	p->set_Destinations (&be_list[0], be_list.size());                 \
	stream->send( p );                                                 \
	stream->flush();                                                   \
}
#endif


#define PRINT_WHERE fprintf(stderr, "%s:%d: ", __FUNCTION__, __LINE__);


/**
 * Receive from a specific stream (blocking) 
 */
#define MRN_STREAM_RECV(stream, tag, data, expected)                                 \
{                                                                                    \
	int rc;                                                                          \
	rc = STREAM_recv(stream, tag, data, true);                                       \
	if (rc == -1)                                                                    \
	{                                                                                \
		PRINT_WHERE;                                                                 \
		fprintf(stderr, "stream::recv() failed (stream_id=%d).",                     \
			STREAM_get_Id(stream));                                                  \
		exit(1);                                                                     \
	}                                                                                \
	if ((expected != TAG_ANY) && (*tag != expected))                                 \
	{                                                                                \
		PRINT_WHERE;                                                                 \
		fprintf(stderr, "stream::recv() tag received %d, but expected %d (%s)\n",    \
			*tag, expected, #expected);                                              \
	}                                                                                \
}


/**
 * Receive from a specific stream (non-blocking) 
 */
#define MRN_STREAM_RECV_NONBLOCKING(stream, tag, data, expected)                     \
{                                                                                    \
	int rc;                                                                          \
	while ((rc = STREAM_recv(stream, tag, data, false)) == 0)                        \
		usleep(500000);                                                              \
	if (rc == -1)                                                                    \
	{                                                                                \
		PRINT_WHERE;                                                                 \
		fprintf(stderr, "stream::recv() failed (stream_id=%d).",                     \
			STREAM_get_Id(stream));                                                  \
		exit(1);                                                                     \
	}                                                                                \
	if ((expected != TAG_ANY) && (*tag != expected))                                 \
	{                                                                                \
		PRINT_WHERE;                                                                 \
		fprintf(stderr, "stream::recv() tag received %d, but expected %d (%s)\n",    \
			*tag, expected, #expected);                                              \
	}                                                                                \
}


/** 
 * Receives from any stream of the network 
 */
#define MRN_NETWORK_RECV(net, tag, data, expected, stream, blocking)                 \
{                                                                                    \
	int rc;                                                                          \
	rc = NETWORK_recv(net, tag, data, stream, blocking);                             \
	if (rc == -1) {                                                                  \
		PRINT_WHERE;                                                                 \
		fprintf(stderr, "network::recv() failed.\n");                                \
		exit(1);                                                                     \
	}                                                                                \
	if ((expected != TAG_ANY) && (*tag != expected)) {                               \
		PRINT_WHERE;                                                                 \
		fprintf(stderr, "network::recv() tag received %d, but expected %d (%s)\n",   \
			*tag, expected, #expected);                                              \
	}                                                                                \
}


/** 
 * Sends message and forces stream to be flushed 
 */
#define MRN_STREAM_SEND(stream, tag, format, args...)                                \
{                                                                                    \
	int rc;                                                                          \
	rc = STREAM_send(stream, tag, format, ## args);                                  \
	if (rc == -1) {                                                                  \
		PRINT_WHERE;                                                                 \
		fprintf(stderr, "stream::send(%s, \"%s\") failed (stream_id=%d, tag=%d).\n", \
			#tag, format, STREAM_get_Id(stream), tag);                               \
		exit(1);                                                                     \
	}                                                                                \
	else {                                                                           \
		rc = STREAM_flush(stream);                                                   \
		if (rc == -1) {                                                              \
			PRINT_WHERE;                                                             \
			fprintf(stderr, "stream::flush() failed (stream_id=%d).\n",              \
				STREAM_get_Id(stream));                                              \
			exit(1);                                                                 \
		}                                                                            \
	}                                                                                \
}

#endif /* __MRNET_WRAPPERS_H__ */


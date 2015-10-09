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

#ifndef __MRNET_TAGS_H__
#define __MRNET_TAGS_H__

#if defined(LIGHTWEIGHT)
 #include <mrnet_lightweight/Types.h>
#else
 #include <mrnet/Types.h>
#endif

/**
 * List here all tags that can be used to send MRNet messages 
 */
typedef enum 
{
   TAG_EXIT=FirstApplicationTag,
   TAG_STREAM,
   TAG_PROT_ID,
   TAG_ACK,
   TAG_ANY
} Tag;

#define FirstProtocolTag TAG_ANY+1

#endif /* __MRNET_TAGS_H__ */

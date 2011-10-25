#ifndef __TAGS_H__
#define __TAGS_H__

#include "MRNet_tags.h"

/* First valid tag for protocols messages starts with FirstProtocolTag */

enum {
   TAG_PING=FirstProtocolTag,
   TAG_PONG
};

#endif /* __TAGS_H__ */


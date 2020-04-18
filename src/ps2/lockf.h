// IvanDSM's dummy lockf
// Work this out later I guess

#include "stdint.h"

#define F_ULOCK 0x0000
#define F_LOCK  0x0001
#define F_TLOCK 0x0002
#define F_TEST  0x0003

int lockf(int fd, int cmd, off_t len)
{

}

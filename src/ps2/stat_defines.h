// IvanDSM's custom stat_defines
// Based on The Open Group Base Specifications Issue 7, 2018 edition
// Just some defines to map standard unix file mode bits to the FIO equivalents

#define S_IRWXU FIO_S_IRWXU
#define S_IRUSR FIO_S_IRUSR
#define S_IWUSR FIO_S_IWUSR
#define S_IXUSR FIO_S_IXUSR
#define S_IRWXG FIO_S_IRWXG
#define S_IRGRP FIO_S_IRGRP
#define S_IWGRP FIO_S_IWGRP
#define S_IXGRP FIO_S_IXGRP
#define S_IRWXO FIO_S_IRWXO
#define S_IROTH FIO_S_IROTH
#define S_IWOTH FIO_S_IWOTH
#define S_IXOTH FIO_S_IXOTH
#define S_ISUID FIO_S_ISUID
#define S_ISGID FIO_S_ISGID
#define S_ISVTX FIO_S_ISVTX

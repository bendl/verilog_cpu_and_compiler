//
// Created by BDL on 19/02/2018.
//

#ifndef LIBPRCO_DBUG_H
#define LIBPRCO_DBUG_H

#define D_INFO  0x1
#define D_ERR 0x2
#define D_WARN  0x4
#define D_ALL   0xFFFF

extern unsigned int g_dbug_level;
extern void dprintf(unsigned int level, char *fmt, ...);
extern void set_dbug_level(unsigned int level);

#endif //LIBPRCO_DBUG_H

//
// Created by BDL on 19/02/2018.
//

#ifndef LIBPRCO_DBUG_H
#define LIBPRCO_DBUG_H

#define D_INFO  0x01
#define D_ERR   0x02
#define D_WARN  0x04
#define D_PARSE 0x08
#define D_GEN   0x10
#define D_EMU   0x20
#define D_EMU2  0x40
#define D_ALL   0xFFFF

extern unsigned int g_dbug_level;
extern void dbprintf(unsigned int level, char *fmt, ...);
extern void set_dbug_level(unsigned int level);

#endif //LIBPRCO_DBUG_H

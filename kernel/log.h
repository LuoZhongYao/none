#ifndef __LOG_H__
#define __LOG_H__

#define DEBUG

#define LOG_PANIC       0 /* system is unavailable */
#define LOG_HW          1 /* hardware error */
#define LOG_ERR         2 /* software error  */
#define LOG_WARNING     3 /* Warning */
#define LOG_NOTICE      4 /* security risks  */
#define LOG_TODO        5 /* todo */
#define LOG_INFO        6 /* informational  */
#define LOG_DEBUG       7 /* debug-level messages */

#define LOG_LEVEL(level)    "PHEWNTID"[level & 7]   
#define LOG_COLOR(level)    "Rrrggrww"[level & 7]

#define LOG(level,own,fmt,...)  printk("\e%c%c/%8s \ew\eO: " fmt,LOG_COLOR(level),LOG_LEVEL(level),own,##__VA_ARGS__)
#define TODO(own,fmt,...)   LOG(LOG_TODO,own,fmt" %s:%d",##__VA_ARGS__,__FILE__,__LINE__)

#ifdef  DEBUG
#   define DBG(own,fmt,...)    LOG(LOG_DEBUG,own,fmt,##__VA_ARGS__)
#   define xFS_DEBUG
#   define xRAMDISK_DEBUG
#   define xSYS_DEBUG
#   define xMM_DEBUG
#   define MP_DEBUG
#   define xPRINT_SCHED
#else
#   define DBG(...)
#endif

#endif

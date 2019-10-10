/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#ifndef OS_H
#define OS_H

/* Figure out which platform we're using */
#if defined(WIN32) || defined(_WIN32)
#   define ESCAPE_CODE "^<ESC^>"
#   ifdef _WIN64
#   else
#   endif
#elif __APPLE__
#   include "TargetConditionals.h"
#   if TARGET_OS_MAC
#   else
#       error "Unknown Apple platform"
#   endif
#elif __linux__
#   define ESCAPE_CODE "\033"
#elif __unix__
    /* have stuff here */
#elif defined(_POSIX_VERSION)
    /* POSIX */
#else
#   error "Unknown compiler"
#endif

#endif /* OS_H */

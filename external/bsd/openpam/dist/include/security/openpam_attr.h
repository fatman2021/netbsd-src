/*	$NetBSD: openpam_attr.h,v 1.5 2014/10/24 18:17:56 christos Exp $	*/

/*
 * Id: openpam_attr.h 656 2013-03-06 22:58:45Z des 
 */

#ifndef SECURITY_OPENPAM_ATTR_H_INCLUDED
#define SECURITY_OPENPAM_ATTR_H_INCLUDED

/* GCC attributes */
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__STRICT_ANSI__)
# define OPENPAM_GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define OPENPAM_GNUC_PREREQ(maj, min) 0
#endif

#if OPENPAM_GNUC_PREREQ(2,5)
# define OPENPAM_FORMAT(params) __attribute__((__format__ params))
#else
# define OPENPAM_FORMAT(params)
#endif

#if OPENPAM_GNUC_PREREQ(3,3)
# define OPENPAM_NONNULL(params) __attribute__((__nonnull__ params))
#else
# define OPENPAM_NONNULL(params)
#endif

#if OPENPAM_GNUC_PREREQ(2,7)
# define OPENPAM_UNUSED(var) var __attribute__((__unused__))
#else
# define OPENPAM_UNUSED(var) var
#endif

#endif /* !SECURITY_OPENPAM_ATTR_H_INCLUDED */

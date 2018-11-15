/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/** @file
 * OpenComal external segment routines header file
 */

#ifndef PDCSEG_H
#define PDCSEG_H

/** SCAN an external segment */
extern void     seg_total_scan(struct seg_des *seg);

/** Load a segment statically, one time */
extern struct seg_des *seg_static_load(struct comal_line *line);

/** Load a segment dynamically, every time the PROC/FUNC is called */
extern struct seg_des *seg_dynamic_load(struct comal_line *line);

/** Free resources associated with a static external segment */
extern struct seg_des *seg_static_free(struct seg_des *seg);

/** Free resources associated with a dynamic external segment */
extern struct seg_des *seg_dynamic_free(struct seg_des *seg);

/** Free all external segments */
extern void     seg_allfree(void);

/** Returns the root of the segment lines, or the current program if seg is NULL */
extern struct comal_line *seg_root(struct seg_des *seg);

#endif

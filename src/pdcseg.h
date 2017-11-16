/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal external segment routines header file */

#ifndef PDCSEG_H
#define PDCSEG_H

extern void seg_total_scan(struct seg_des *seg);
extern struct seg_des *seg_static_load(struct comal_line *line);
extern struct seg_des *seg_dynamic_load(struct comal_line *line);
extern struct seg_des *seg_static_free(struct seg_des *seg);
extern struct seg_des *seg_dynamic_free(struct seg_des *seg);
extern void seg_allfree(void);
extern struct comal_line *seg_root(struct seg_des *seg);

#endif

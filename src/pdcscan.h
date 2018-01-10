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
 * OpenComal program management
 */

#ifndef PDCSCAN_H
#define PDCSCAN_H

extern bool scan_scan(struct seg_des *seg, char *errtxt,
		     struct comal_line **errline);
extern void prog_structure_scan(void);
extern int scan_nescessary(struct comal_line *line);
extern bool assess_scan(struct comal_line *line);

#endif

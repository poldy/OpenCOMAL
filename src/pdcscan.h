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

/** SCAN the current program */
extern bool scan_scan(struct seg_des *seg, char *errtxt,
		     struct comal_line **errline);

/** Partial SCAN to get the indentation right */
extern void prog_structure_scan(void);

/** Tests if the line @c line contains a structure command, which forces a SCAN if changed */
extern int scan_necessary(struct comal_line *line);

/** Assess whether a SCAN will be required after editing the line @c line */
extern bool assess_scan(struct comal_line *line);

#endif

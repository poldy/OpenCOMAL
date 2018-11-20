/** @file
 * Configure GNU nana to integrate with OpenCOMAL.
 */

#ifndef PDCNANA_H
#define PDCNANA_H

#define I_DEFAULT_HANDLER(e,f,l,p) fatal(p)
#define I_DEFAULT_PARAMS "assert"
#define L_DEFAULT_HANDLER my_printf
#define L_DEFAULT_PARAMS MSG_DEBUG
#include "nana.h"

#endif

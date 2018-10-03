/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>

#include <tfblib/tfblib.h>
#include <tfblib/tfb_kb.h>

#include "utils.h"

double int_pow(double b, int p)
{
   double ret = 1.0;

   for (int i = 0; i < p; i++)
      ret *= b;

   return ret;
}

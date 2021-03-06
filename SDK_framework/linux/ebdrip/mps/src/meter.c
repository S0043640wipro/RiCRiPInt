/* impl.c.meter: METERS
 *
 * $Id: meter.c,v 1.14.11.1.1.1 2013/12/19 11:27:04 anon Exp $
 * $HopeName: MMsrc!meter.c(EBDSDK_P.1) $
 * Copyright (c) 2001 Ravenbrook Limited.
 * Copyright (C) 2007 Global Graphics Software Ltd. All rights reserved.
 *Global Graphics Software Ltd. Confidential Information.
 *
 * TRANSGRESSIONS
 *
 * .trans.label: We label meters with EventLabelAddr, but of course that's
 * meant for labelling Addr's.  We get away with it as long as the type
 * Meter is compatible with Addr.
 */

#include "meter.h"
#include "mpm.h"

SRCID(meter, "$Id: meter.c,v 1.14.11.1.1.1 2013/12/19 11:27:04 anon Exp $");


/* MeterInit -- initialize a meter */

void MeterInit(Meter meter, Word name, void *owner)
{
  meter->count = 0;
  meter->total = 0.0;
  meter->meanSquared = 0.0;
  meter->max = 0;
  meter->min = (Size)-1;

  EventLabelAddr((Addr)meter, name); /* see .trans.label */
  EVENT_PP(MeterInit, meter, owner);
  UNUSED(owner); /* @@@@ hack */
}


/* MeterAccumulate -- accumulate another data point in the meter */

void MeterAccumulate(Meter meter, Size amount)
{
  Count count = meter->count + 1;
  double total = meter->total;
  double meanSquared = meter->meanSquared;
  double dcount = (double)count;

  /* .limitation.variance: This computation accumulates a running
   * mean^2, minimizing overflow, but sacrificing numerical stablity
   * for small variances.  For more accuracy, the data set should be
   * emitted using a telemetry stream and analyzed off-line.
    .stddev: stddev = sqrt(meanSquared - mean^2).
   */
  meter->count = count;
  meter->total = total + amount;
  meter->meanSquared =
    meanSquared / dcount * (dcount - 1.0)
    + amount / dcount * amount;
  if (amount > meter->max)
    meter->max = amount;
  if (amount < meter->min)
    meter->min = amount;
}


/* MeterWrite -- describe method for meters */

Res MeterWrite(Meter meter, mps_lib_FILE *stream)
{
  Res res = ResOK;

  res = WriteF(stream,
               "meter {",
               "count: $U", meter->count,
               NULL);
  if (res != ResOK)
    return res;
  if (meter->count > 0) {
    double mean = meter->total / (double)meter->count;
    
    res = WriteF(stream,
                 ", total: $D", meter->total,
                 ", max: $U", meter->max,
                 ", min: $U", meter->min,
                 ", mean: $D", mean,
                 ", mean^2: $D", meter->meanSquared,
                 NULL);
    if (res != ResOK)
      return res;
  }
  res = WriteF(stream, "}\n", NULL);

  return res;
}


/* MeterEmit -- emit an evnet with the current data from the meter */

void MeterEmit(Meter meter)
{
  EVENT_PDDWWW(MeterValues, meter, meter->total, meter->meanSquared,
               meter->count, meter->max, meter->min);
  UNUSED(meter); /* @@@@ hack */
}

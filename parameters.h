
#ifndef BFC_PARAMETERS_H
#define BFC_PARAMETERS_H

typedef enum {
  /* Let the architecture decide. */
  OVERFLOW_BEHAVIOR_UNDEFINED,
  /* Check and contain overflows. */
  OVERFLOW_BEHAVIOR_CAP,
  /* Check and abort on overflow. */
  OVERFLOW_BEHAVIOR_ABORT,
} OverflowBehavior;

typedef struct {
  int overflow_behavior;
  /* Size in sizeof() units. */
  int byte_size;
} Parameters;

extern Parameters G_PARAMETERS;

#endif /* define BFC_PARAMETERS_H */



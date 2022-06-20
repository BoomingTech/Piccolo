#pragma once

#define PICCOLO_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define PICCOLO_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define PICCOLO_PIN(a, min_value, max_value) PICCOLO_MIN(max_value, PICCOLO_MAX(a, min_value))

#define PICCOLO_VALID_INDEX(idx, range) (((idx) >= 0) && ((idx) < (range)))
#define PICCOLO_PIN_INDEX(idx, range) PICCOLO_PIN(idx, 0, (range)-1)

#define PICCOLO_SIGN(x) ((((x) > 0.0f) ? 1.0f : 0.0f) + (((x) < 0.0f) ? -1.0f : 0.0f))

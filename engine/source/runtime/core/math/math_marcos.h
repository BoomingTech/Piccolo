#pragma once

#define PILOT_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define PILOT_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define PILOT_PIN(a, min_value, max_value) PILOT_MIN(max_value, PILOT_MAX(a, min_value))

#define PILOT_VALID_INDEX(idx, range) (((idx) >= 0) && ((idx) < (range)))
#define PILOT_PIN_INDEX(idx, range) PILOT_PIN(idx, 0, (range)-1)

#define PILOT_SIGN(x) ((((x) > 0.0f) ? 1.0f : 0.0f) + (((x) < 0.0f) ? -1.0f : 0.0f))

#pragma once

#define POILT_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define POILT_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define POILT_PIN(a, min_value, max_value) POILT_MIN(max_value, POILT_MAX(a, min_value))

#define POILT_VALID_INDEX(idx, range) (((idx) >= 0) && ((idx) < (range)))
#define POILT_PIN_INDEX(idx, range) POILT_PIN(idx, 0, (range)-1)

#define POILT_SIGN(x) ((((x) > 0.0f) ? 1.0f : 0.0f) + (((x) < 0.0f) ? -1.0f : 0.0f))

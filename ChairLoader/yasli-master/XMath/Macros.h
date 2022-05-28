#pragma once

#ifdef FOR_EACH
# undef FOR_EACH
#endif
#define FOR_EACH(list, iterator, ...) \
	for(__VA_ARGS__ iterator = (list).begin(); iterator != (list).end(); ++iterator)


#ifdef ARRAY_LEN
# undef ARRAY_LEN
#endif
#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

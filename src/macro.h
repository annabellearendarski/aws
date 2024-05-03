#ifndef MACRO_H
#define MACRO_H

#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr)	\
	(type *)((char *)(ptr) - offsetof(type, member))})

#define structof(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#endif /* MACRO_H */

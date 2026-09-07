#ifndef BVR_INTERNAL_COMMON_H
#define BVR_INTERNAL_COMMON_H

#define PTR_ADD(ptr, offset) \
    ((void *)((intptr_t)(ptr) + (intptr_t)(offset)))

#define CLIP(value, min, max) \
    ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#endif /* BVR_INTERNAL_COMMON_H */

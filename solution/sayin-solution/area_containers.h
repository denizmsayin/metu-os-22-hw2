#ifndef HW2_AREA_CONTAINERS_H_
#define HW2_AREA_CONTAINERS_H_

#include <stddef.h>

#include "area.h"
#include "arraylist.h"

// An abstract container for areas. I need both a dict-style and 
// stand-alone version (map & set).

// Carefully define the used arraylist types

#ifndef ARRAYLIST_AREA_DEFINED
#define ARRAYLIST_AREA_DEFINED
DEFINE_ARRAYLIST(struct area, area)
#endif

typedef struct {
   arraylist_area_t area_list;
} area_container_t;

void ac_init(area_container_t *);
void ac_destroy(area_container_t *);

// To keep things abstract, we return pointers from find and use them for removal too,
// like simplified iterators.
void ac_insert(area_container_t *, struct area);
void ac_remove(area_container_t *, struct area *);

struct area *ac_find_intersection(const area_container_t *, struct area);
struct area *ac_find_by_area(const area_container_t *, struct area);

// Repeats for <area, value> map-style version. Values are void * and the
// map assumes no ownership of them to keep things very simple.

#ifndef ARRAYLIST_VP_DEFINED
#define ARRAYLIST_VP_DEFINED
DEFINE_ARRAYLIST(void *, vp)
#endif

typedef struct {
    arraylist_area_t area_list;
    arraylist_vp_t values;
} area_keyed_map_t;

void akm_init(area_keyed_map_t *);
void akm_destroy(area_keyed_map_t *);

#ifndef ARRAYLIST_AREAPTR_DEFINED
#define ARRAYLIST_AREAPTR_DEFINED
DEFINE_ARRAYLIST(struct area *, areaptr)
#endif

void akm_insert(area_keyed_map_t *, struct area, void *value);
void akm_remove(area_keyed_map_t *, struct area *);

arraylist_areaptr_t akm_find_intersections(const area_keyed_map_t *, struct area);
struct area *akm_find_by_vptr(const area_keyed_map_t *, void *);
void *akm_find_by_area(const area_keyed_map_t *, struct area);
void *akm_get_value(const area_keyed_map_t *, struct area *);

// Iterator for akm contents
#define AKM_VBEGIN(akm) (ALIST_BEGIN(&(akm)->values))
#define AKM_VEND(akm) (ALIST_END(&(akm)->values))

// Note: Would be nice if I could key these connections by thread id, but
// that is a lot of extra effort. Instead, I can remove from akm via the void *
// values, which will be unique, and remove from areas with just the areas,
// since their uniqueness is not important.

#endif

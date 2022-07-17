#include "area_containers.h"

#include <stdlib.h>
#include <assert.h>

#include "arraylist.h"

#define INITIAL_CAP 8

void ac_init(area_container_t *ac)
{
    arraylist_area_init(&ac->area_list, 0);
}

void ac_destroy(area_container_t *ac)
{
    arraylist_area_destroy(&ac->area_list);
}

// Standart O(1) append at end
void ac_insert(area_container_t *ac, struct area a)
{
    arraylist_area_insert(&ac->area_list, a);
}

// O(1) quick-swap with last element
void ac_remove(area_container_t *ac, struct area *a)
{
    arraylist_area_t *list = &ac->area_list;
    size_t ridx = arraylist_area_itr2idx(list, a);
    arraylist_area_qremove(list, ridx);
}

struct area *ac_find_intersection(const area_container_t *ac, struct area a)
{
    const arraylist_area_t *list = &ac->area_list;
    for (struct area *itr = ALIST_BEGIN(list); itr != ALIST_END(list); itr++)
        if (areas_intersect(a, *itr))
            return itr;
    return NULL;
}

struct area *ac_find_by_area(const area_container_t *ac, struct area a)
{
    const arraylist_area_t *list = &ac->area_list;
    for (struct area *itr = ALIST_BEGIN(list); itr != ALIST_END(list); itr++)
        if (areas_equal(a, *itr))
            return itr;
    return NULL;
}

// area_keyed_map functions

void akm_init(area_keyed_map_t *akm)
{
    arraylist_area_init(&akm->area_list, 0);
    arraylist_vp_init(&akm->values, 0);
}

void akm_destroy(area_keyed_map_t *akm)
{
    arraylist_area_destroy(&akm->area_list);
    arraylist_vp_destroy(&akm->values);
}

void akm_insert(area_keyed_map_t *akm, struct area a, void *value)
{
    arraylist_area_insert(&akm->area_list, a);
    arraylist_vp_insert(&akm->values, value);
}

void akm_remove(area_keyed_map_t *akm, struct area *a)
{
    arraylist_area_t *alist = &akm->area_list;
    size_t ridx = arraylist_area_itr2idx(alist, a);
    arraylist_area_qremove(alist, ridx);
    arraylist_vp_qremove(&akm->values, ridx);
}

arraylist_areaptr_t akm_find_intersections(const area_keyed_map_t *akm, struct area a)
{
    const arraylist_area_t *alist = &akm->area_list;
    arraylist_areaptr_t intersecting_areas; 
    arraylist_areaptr_init(&intersecting_areas, 0);
    for (struct area *itr = ALIST_BEGIN(alist); itr != ALIST_END(alist); itr++)
        if (areas_intersect(a, *itr))
            arraylist_areaptr_insert(&intersecting_areas, itr);
    return intersecting_areas;
}

void *akm_find_by_area(const area_keyed_map_t *akm, struct area a)
{
    const arraylist_area_t *alist = &akm->area_list;
    for (struct area *itr = ALIST_BEGIN(alist); itr != ALIST_END(alist); itr++) {
        if (areas_equal(a, *itr)) {
            size_t i = arraylist_area_itr2idx(&akm->area_list, itr);
            return *ALIST_AT(&akm->values, i);
        }
    }
    return NULL;
}

struct area *akm_find_by_vptr(const area_keyed_map_t *akm, void *value)
{
    for (void **itr = ALIST_BEGIN(&akm->values); itr != ALIST_END(&akm->values); itr++) {
        if (*itr == value) {
            size_t i = arraylist_vp_itr2idx(&akm->values, itr);
            return ALIST_AT(&akm->area_list, i);
        }
    }
    return NULL;
}

void *akm_get_value(const area_keyed_map_t *akm, struct area *a)
{
    size_t i = arraylist_area_itr2idx(&akm->area_list, a);
    return *ALIST_AT(&akm->values, i);
}


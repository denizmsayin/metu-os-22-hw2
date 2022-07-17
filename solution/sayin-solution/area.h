#ifndef HW2_AREA_H_
#define HW2_AREA_H_

// Fits in one 8-byte register :)
struct area {
    short tli, tlj; // top-left
    short bri, brj; // bottom-right
};

static inline struct area area_from_span(short tli, short tlj, short si, short sj)
{
    return (struct area) { tli, tlj, tli + si - 1, tlj + sj - 1 };
}

static inline struct area area_3x3_from_center(short ci, short cj)
{
    return (struct area) { ci - 1, cj - 1, ci + 1, cj + 1};
}

static inline int intersects_1d_(short s1, short e1, short s2, short e2)
{
    return s1 <= e2 && s2 <= e1;
}

static inline int areas_intersect(struct area a1, struct area a2)
{
    return intersects_1d_(a1.tli, a1.bri, a2.tli, a2.bri) &&
           intersects_1d_(a1.tlj, a1.brj, a2.tlj, a2.brj);
}

static inline int areas_equal(struct area a1, struct area a2)
{
    return a1.bri == a2.bri && a1.brj == a2.brj && a1.tli == a2.tli && a1.tlj == a2.tlj;
}

#endif

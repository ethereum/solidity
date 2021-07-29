// implicit conversions
fixed128x2 constant a = fixed64x1(0);
fixed128x1 constant b = fixed64x1(0);
fixed128x1 constant c = ufixed64x1(0);
fixed128x3 constant d = ufixed64x1(0);
// explicit conversions
// precision reduction
ufixed64x1 constant r = ufixed64x1(ufixed64x2(0));
// sign change
ufixed64x2 constant s = ufixed64x2(fixed64x2(0));
// bit reduction
fixed32x2 constant t = fixed32x2(fixed64x2(0));
// ----

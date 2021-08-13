// implicit conversions
// precision reduction
fixed128x1 constant a = fixed64x2(0);
// sign change
fixed64x1 constant b = ufixed64x1(0);
ufixed256x3 constant c = fixed64x1(0);
// width reduction
fixed32x1 constant d = ufixed64x1(0);

// explicit conversions
// precision and sign
fixed256x1 constant e = fixed256x1(ufixed64x2(0));
// precision and value range
ufixed32x1 constant f = ufixed32x1(ufixed64x2(0));
// value range and sign
fixed32x2 constant g = fixed32x2(ufixed64x2(0));
ufixed32x2 constant h = ufixed32x2(fixed64x2(0));
// ----
// TypeError 7407: (71-83): Type fixed64x2 is not implicitly convertible to expected type fixed128x1. Conversion would incur precision loss - use explicit conversion instead.
// TypeError 7407: (123-136): Type ufixed64x1 is not implicitly convertible to expected type fixed64x1.
// TypeError 7407: (163-175): Type fixed64x1 is not implicitly convertible to expected type ufixed256x3.
// TypeError 7407: (219-232): Type ufixed64x1 is not implicitly convertible to expected type fixed32x1.
// TypeError 9640: (305-330): Explicit type conversion not allowed from "ufixed64x2" to "fixed256x1". Can only change one of precision, number of bits and signedness at the same time.
// TypeError 9640: (385-410): Explicit type conversion not allowed from "ufixed64x2" to "ufixed32x1". Can only change one of precision, number of bits and signedness at the same time.
// TypeError 9640: (459-483): Explicit type conversion not allowed from "ufixed64x2" to "fixed32x2". Can only change one of precision, number of bits and signedness at the same time.
// TypeError 9640: (509-533): Explicit type conversion not allowed from "fixed64x2" to "ufixed32x2". Can only change one of precision, number of bits and signedness at the same time.

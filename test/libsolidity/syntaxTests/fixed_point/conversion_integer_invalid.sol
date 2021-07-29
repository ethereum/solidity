ufixed16x1 constant x = ufixed16x1(uint16(0));
ufixed64x1 constant y = ufixed64x1(int8(0));
// ----
// TypeError 9640: (24-45): Explicit type conversion not allowed from "uint16" to "ufixed16x1".
// TypeError 9640: (71-90): Explicit type conversion not allowed from "int8" to "ufixed64x1".

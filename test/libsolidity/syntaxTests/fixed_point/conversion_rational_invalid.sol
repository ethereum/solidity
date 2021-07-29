// implicit conversions
fixed64x2 constant a = 1.123;
ufixed64x2 constant b = -1.123;
fixed64x28 constant c = 1;
fixed64x28 constant d = -1;
ufixed256x77 constant e = 1/3;
ufixed256x77 constant f = -1;

// explicit conversions
fixed64x2 constant g = fixed64x2(2**64);
ufixed64x2 constant h = ufixed64x2(-1);

// ----
// TypeError 2326: (47-52): Type rational_const 1123 / 1000 is not implicitly convertible to expected type fixed64x2. Try converting to type ufixed16x3 or use an explicit conversion.
// TypeError 2326: (78-84): Type rational_const -1123 / 1000 is not implicitly convertible to expected type ufixed64x2. Try converting to type fixed16x3 or use an explicit conversion.
// TypeError 7407: (110-111): Type int_const 1 is not implicitly convertible to expected type fixed64x28.
// TypeError 7407: (137-139): Type int_const -1 is not implicitly convertible to expected type fixed64x28.
// TypeError 4426: (167-170): Type rational_const 1 / 3 is not implicitly convertible to expected type ufixed256x77, but it can be explicitly converted.
// TypeError 7407: (198-200): Type int_const -1 is not implicitly convertible to expected type ufixed256x77.
// TypeError 9640: (250-266): Explicit type conversion not allowed from "int_const 18446744073709551616" to "fixed64x2". Value is too large.
// TypeError 9640: (292-306): Explicit type conversion not allowed from "int_const -1" to "ufixed64x2". Value is too small.

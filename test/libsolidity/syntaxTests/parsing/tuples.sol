contract C {
	function f() {
		uint a = (1);
		var (b,) = (1,);
		var (c,d) = (1, 2 + a);
		var (e,) = (1, 2, b);
		(a) = 3;
	}
}
// ----
// Warning: (52-53): Use of the "var" keyword is deprecated.
// Warning: (71-72): Use of the "var" keyword is deprecated.
// Warning: (73-74): Use of the "var" keyword is deprecated.
// Warning: (97-98): Use of the "var" keyword is deprecated.
// Warning: (47-62): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (47-62): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (66-88): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (92-112): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (92-112): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (14-127): No visibility specified. Defaulting to "public". 
// Warning: (71-72): Unused local variable.
// Warning: (73-74): Unused local variable.
// Warning: (97-98): Unused local variable.
// Warning: (14-127): Function state mutability can be restricted to pure

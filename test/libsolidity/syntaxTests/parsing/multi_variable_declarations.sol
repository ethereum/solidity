contract C {
	function f() {
		var (a,b,c) = g();
		var (d) = 2;
		var (,e) = 3;
		var (f,) = 4;
		var (x,,) = g();
		var (,y,) = g();
		var () = g();
		var (,,) = g();
	}
	function g() returns (uint, uint, uint) {}
}
// ----
// Warning: (36-37): Use of the "var" keyword is deprecated.
// Warning: (38-39): Use of the "var" keyword is deprecated.
// Warning: (40-41): Use of the "var" keyword is deprecated.
// Warning: (57-58): Use of the "var" keyword is deprecated.
// Warning: (73-74): Use of the "var" keyword is deprecated.
// Warning: (88-89): Use of the "var" keyword is deprecated.
// Warning: (104-105): Use of the "var" keyword is deprecated.
// Warning: (124-125): Use of the "var" keyword is deprecated.
// Warning: (88-89): This declaration shadows an existing declaration.
// Warning: (52-63): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (67-79): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (67-79): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (83-95): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (83-95): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
// TypeError: (137-149): Too many components (3) in value for variable assignment (0) needed

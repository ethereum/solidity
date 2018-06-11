contract C {
	function f() pure public {
		(uint a, uint b, uint c) = g();
		(uint d) = 2;
		(, uint e) = 3;
		(uint h,) = 4;
		(uint x,,) = g();
		(, uint y,) = g();
        a; b; c; d; e; h; x; y;
	}
	function g() pure public returns (uint, uint, uint) {}
}
// ----
// Warning: (93-107): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (111-124): Different number of components on the left hand side (2) than on the right hand side (1).

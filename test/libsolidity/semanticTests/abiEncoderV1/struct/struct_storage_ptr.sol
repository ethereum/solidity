library L {
	struct S { uint x; uint y; }
	function f(uint[] storage r, S storage s) public returns (uint, uint, uint, uint) {
		r[2] = 8;
		s.x = 7;
		return (r[0], r[1], s.x, s.y);
	}
}
contract C {
	uint8 x = 3;
	L.S s;
	uint[] r;
	function f() public returns (uint, uint, uint, uint, uint, uint) {
		r = new uint[](6);
		r[0] = 1;
		r[1] = 2;
		r[2] = 3;
		s.x = 11;
		s.y = 12;
		(uint a, uint b, uint c, uint d) = L.f(r, s);
		return (r[2], s.x, a, b, c, d);
	}
}
// ====
// compileViaYul: also
// ----
// library: L
// f() -> 8, 7, 1, 2, 7, 12
// gas irOptimized: 167580
// gas legacy: 169475
// gas legacyOptimized: 167346

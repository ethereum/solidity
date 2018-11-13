contract C {
	function f() public pure {
		uint a = (1);
		(uint b,) = (1,2);
		(uint c, uint d) = (1, 2 + a);
		(uint e,) = (1, b);
		(a) = 3;
		a;b;c;d;e;
	}
}
// ----

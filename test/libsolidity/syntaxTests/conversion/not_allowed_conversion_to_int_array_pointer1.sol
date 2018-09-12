contract C {
	uint[] a;
	uint[] b;
	function f() public view {
		uint[] storage c = a;
		uint[] storage d = b;
		d = uint[](c);
	}
}
// ----

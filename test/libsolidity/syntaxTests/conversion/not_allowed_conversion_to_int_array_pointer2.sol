contract C {
	uint[] a;
	uint[] b;
	function f() public view {
		uint[] storage c = a;
		uint[] memory d = b;
		d = uint[](c);
	}
}
// ----

contract C {
	function f() public pure {
		uint a;
		(a,) = (uint(1),);
	}
}
// ----
// TypeError: (60-70): Tuple component cannot be empty.

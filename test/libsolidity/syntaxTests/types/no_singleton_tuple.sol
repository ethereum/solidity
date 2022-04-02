contract C {
	function f() public pure {
		uint a;
		(a,) = (uint(1),);
	}
}
// ----
// TypeError 8381: (60-70='(uint(1),)'): Tuple component cannot be empty.

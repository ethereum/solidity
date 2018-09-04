contract C {
	function f() public pure returns (uint, uint, bytes32) {
		uint a;
		bytes32 b;
		(a,) = f();
		(,b) = f();
	}
}
// ----
// Warning: (96-106): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (110-120): Different number of components on the left hand side (2) than on the right hand side (3).

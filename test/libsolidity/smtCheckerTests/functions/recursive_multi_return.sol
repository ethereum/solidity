pragma experimental SMTChecker;

contract C {
	function g() public pure returns (uint, uint) {
		uint a;
		uint b;
		(a, b) = g();
	}
}
//
// ----

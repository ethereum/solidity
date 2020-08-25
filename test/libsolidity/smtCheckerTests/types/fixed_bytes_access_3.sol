pragma experimental SMTChecker;
contract C {
	bytes16[][] a;
	function g() internal view returns (bytes16[] storage) {
		return a[2];
	}
	function h() internal view returns (bytes16) {
		return a[2][2];
	}
	function f() external view {
		g()[3][4];
		h()[5];
	}
}
// ----
// Warning 7989: (238-247): Assertion checker does not yet support index accessing fixed bytes.
// Warning 7989: (251-257): Assertion checker does not yet support index accessing fixed bytes.

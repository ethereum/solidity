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

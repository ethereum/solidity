pragma experimental SMTChecker;

contract C {
	function f(bytes calldata x) external pure {
		x[:18726387213];
		x[18726387213:];
		x[18726387213:111111111111111111];
	}
}
// ----

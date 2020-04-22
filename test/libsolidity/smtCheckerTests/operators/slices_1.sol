pragma experimental SMTChecker;

contract C {
	function f(bytes calldata x) external pure {
		x[:18726387213];
		x[18726387213:];
		x[18726387213:111111111111111111];
	}
}
// ----
// Warning: (94-109): Assertion checker does not yet implement this expression.
// Warning: (113-128): Assertion checker does not yet implement this expression.
// Warning: (132-165): Assertion checker does not yet implement this expression.

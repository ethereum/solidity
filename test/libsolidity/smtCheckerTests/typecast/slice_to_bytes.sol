pragma experimental SMTChecker;

contract C {
	function f(bytes calldata x) external pure {
		bytes(x[:18726387213]);
		bytes(x[18726387213:]);
		bytes(x[18726387213:111111111111111111]);
	}
}
// ----
// Warning 2923: (100-115): Assertion checker does not yet implement this expression.
// Warning 2923: (126-141): Assertion checker does not yet implement this expression.
// Warning 2923: (152-185): Assertion checker does not yet implement this expression.

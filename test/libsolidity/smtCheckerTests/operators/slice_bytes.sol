pragma experimental SMTChecker;
contract C {
	function f(bytes calldata b) external pure {
		((b[:])[5]);
	}
}

pragma experimental SMTChecker;

contract C
{
	function f(bytes memory b) public pure returns (bytes memory) {
		bytes memory c = b;
		return b;
	}
}
// ----
// Warning: (113-127): Unused local variable.

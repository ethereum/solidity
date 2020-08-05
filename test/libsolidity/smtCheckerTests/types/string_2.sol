pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		string memory s = "Hello World";
	}
}
// ----
// Warning 2072: (76-91): Unused local variable.

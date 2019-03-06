pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		string memory s = "Hello World";
	}
}
// ----
// Warning: (76-91): Unused local variable.
// Warning: (94-107): Assertion checker does not yet support the type of this literal (literal_string "Hello World").

contract C
{
	function f() public pure {
		string memory s = "Hello World";
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (43-58): Unused local variable.

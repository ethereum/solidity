contract C
{
	function f(uint x) public pure {
		uint data;
		while (data < x)
			uint data2 = 2;
	}
}
// ----
// Warning: (82-92): Unused local variable.

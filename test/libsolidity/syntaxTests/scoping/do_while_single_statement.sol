contract C
{
	function f(uint x) public pure {
		uint data;
		do
			uint data2 = 2;
		while (data < x);
	}
}
// ----
// Warning: (68-78): Unused local variable.

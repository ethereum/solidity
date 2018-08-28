contract C
{
	function f(uint x) public pure {
		int data;
		if (x > 7)
			int data2 = 1;
		else
			data = 1;
	}
}
// ----
// Warning: (75-84): Unused local variable.

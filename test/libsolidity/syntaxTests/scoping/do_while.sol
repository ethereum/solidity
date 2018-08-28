contract C
{
	function f(uint x) public pure {
		uint data;
		do {
			uint data2 = 1;
			data += data2;
		} while (data < x);
	}
}

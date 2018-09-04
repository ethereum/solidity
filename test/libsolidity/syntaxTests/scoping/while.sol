contract C
{
	function f(uint x) public pure {
		uint data;
		while (data < x) {
			uint data2 = 1;
			data += data2;
		}
	}
}

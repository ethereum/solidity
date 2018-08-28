contract C
{
	function f(uint x) public pure {
		int data;
		if (x > 7) {
			int data2 = 1;
			int data3 = 2;
			data = data2 + data3;
		}
		else
			data = 1;
	}
}

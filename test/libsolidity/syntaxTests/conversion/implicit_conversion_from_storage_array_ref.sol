contract C {
	int[10] x;
	int[] y;
	function f() public {
		y = x;
	}
}

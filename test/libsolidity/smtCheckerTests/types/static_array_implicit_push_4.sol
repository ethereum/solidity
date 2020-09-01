pragma experimental SMTChecker;
contract D {
	int16[] inner;
	int[][] data;
	function t() public {
		data.push(inner);
	}
}


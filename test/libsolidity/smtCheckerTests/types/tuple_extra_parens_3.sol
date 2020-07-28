pragma experimental SMTChecker;
contract C {
	function f2() public pure returns(int) {
		int a;
		(((((((, a),)))))) = ((1, 2), 3);
	}
}

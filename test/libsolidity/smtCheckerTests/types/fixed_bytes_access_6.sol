pragma experimental SMTChecker;

contract C {
	function f() public pure {
		bytes4 x = 0x01020304;
		bytes1 b = x[3];
		assert(b == b[0]);
		assert(b == b[0][0]);
		assert(b == b[0][0][0][0][0][0][0][0][0][0][0]);
	}
}

pragma experimental SMTChecker;

contract C {
	function f(bytes32 _x) public pure {
		require(_x != "test");
		bytes32 y = _x;
		bytes32 z = _x;
		assert(z == "test");
		assert(y != "testx");
	}
}
// ----
// Warning 6328: (147-166): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 52647538817385212172903286807934654968315727694643370704309751478220717293569\n\n\nTransaction trace:\nconstructor()\nf(52647538817385212172903286807934654968315727694643370704309751478220717293569)
// Warning 6328: (170-190): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 52647538830022687173130149211684818290356179572910782152375644828738034597888\n\n\nTransaction trace:\nconstructor()\nf(52647538830022687173130149211684818290356179572910782152375644828738034597888)

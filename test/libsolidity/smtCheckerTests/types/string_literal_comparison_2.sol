contract C {
	function f(bytes32 _x) public pure {
		require(_x != "test");
		bytes32 y = _x;
		bytes32 z = _x;
		assert(z == "test");
		assert(y != "testx");
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (114-133): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 0\ny = 0\nz = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
// Warning 6328: (137-157): CHC: Assertion violation happens here.\nCounterexample:\n\n_x = 52647538830022687173130149211684818290356179572910782152375644828738034597888\ny = 52647538830022687173130149211684818290356179572910782152375644828738034597888\nz = 52647538830022687173130149211684818290356179572910782152375644828738034597888\n\nTransaction trace:\nC.constructor()\nC.f(52647538830022687173130149211684818290356179572910782152375644828738034597888)

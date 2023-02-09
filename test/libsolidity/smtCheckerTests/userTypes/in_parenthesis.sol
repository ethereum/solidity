type MyInt is int;
contract C {
	function f() public pure returns (MyInt a) {
		a = MyInt.wrap(5);
		assert(MyInt.unwrap(a) == 5);
		assert(MyInt.unwrap(a) == 6); // should fail
	}

	function g() public pure {
		MyInt x = f();
		assert(MyInt.unwrap(x) == 5);
		assert(MyInt.unwrap(x) == 6); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (133-161): CHC: Assertion violation happens here.
// Warning 6328: (261-289): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

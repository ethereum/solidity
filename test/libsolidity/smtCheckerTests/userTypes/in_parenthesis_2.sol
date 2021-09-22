type MyInt is int;
contract C {
	function f() public pure returns (MyInt a, int b) {
		(MyInt).wrap;
		a = (MyInt).wrap(5);
		(MyInt).unwrap;
		b = (MyInt).unwrap((MyInt).wrap(10));
	}

	function g() public pure {
		(MyInt x, int y) = f();
		assert(MyInt.unwrap(x) == 5);
		assert(MyInt.unwrap(x) == 6); // should fail
		assert(y == 10);
		assert(y == 11); // should fail
	}

}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6133: (87-99): Statement has no effect.
// Warning 6133: (126-140): Statement has no effect.
// Warning 6328: (274-302): CHC: Assertion violation happens here.
// Warning 6328: (340-355): CHC: Assertion violation happens here.

contract C {
	bytes1 public x;
	bytes3 public y;

	function f() public view {
		bytes1 a = this.x();
		bytes3 b = this.y();
		assert(a == x); // should hold
		assert(a == 'a'); // should fail
		assert(b == y); // should hold
		assert(y == "abc"); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (159-175): CHC: Assertion violation happens here.
// Warning 6328: (227-245): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

contract C {

	struct S {
		uint x;
		int y;
	}

	function g() public pure returns (bool b, S memory s) {
		b = true;
		s.x = 42;
		s.y = -1;
	}

	function f() public view {
		bool success = false;
		try this.g() returns (bool b, S memory s) {
			success = true;
			assert(b && s.x == 42 && s.y == -1);
		} catch {
		}
		assert(success); // fails, not guaranteed that there will be no error
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (321-336): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

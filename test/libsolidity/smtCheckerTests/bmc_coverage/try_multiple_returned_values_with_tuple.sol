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
			assert(b && s.x == 42 && s.y == -1); // should hold
		} catch {
		}
		assert(success); // fails, not guaranteed that there will be no error
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (336-351): BMC: Assertion violation happens here.

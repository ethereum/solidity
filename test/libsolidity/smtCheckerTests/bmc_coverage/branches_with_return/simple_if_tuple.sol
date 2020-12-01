pragma experimental SMTChecker;

contract C {

	uint x;
	uint y;

	function check() public {
		require(x == 0);
		require(y == 0);
		conditional_increment();
		assert(x == 0); // should fail;
		assert(x == 1); // should fail;
		assert(x == 2); // should hold;
	}

	function conditional_increment() internal {
		if (x == 0) {
			(x,y) = (2,2);
			return;
		}
		(x,y) = (1,1);
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (160-174): BMC: Assertion violation happens here.
// Warning 4661: (194-208): BMC: Assertion violation happens here.

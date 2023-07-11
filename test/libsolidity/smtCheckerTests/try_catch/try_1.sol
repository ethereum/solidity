contract C {
	int x;

	function g() public  {
		x = 42;
	}

	function f() public {
		x = 0;
		bool success = false;
		try this.g() {
			success = true;
		} catch (bytes memory s) {
			assert(x == 0); // should hold
		}
		assert(success); // fails for now, since external call is over-approximated (both success and fail are considered possible) for now even for known code
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5667: (163-177): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (221-236): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

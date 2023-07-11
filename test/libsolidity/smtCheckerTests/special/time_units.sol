contract D {
	function f() public pure {
		assert(1 == 1 seconds);
		assert(2 == 1 seconds);
		assert(2 minutes == 120 seconds);
		assert(3 minutes == 120 seconds);
		assert(2 hours == 120 minutes);
		assert(3 hours == 120 minutes);
		assert(2 days == 48 hours);
		assert(4 days == 48 hours);
		assert(2 weeks == 14 days);
		assert(25 weeks == 14 days);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (69-91): CHC: Assertion violation happens here.
// Warning 6328: (131-163): CHC: Assertion violation happens here.
// Warning 6328: (201-231): CHC: Assertion violation happens here.
// Warning 6328: (265-291): CHC: Assertion violation happens here.
// Warning 6328: (325-352): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

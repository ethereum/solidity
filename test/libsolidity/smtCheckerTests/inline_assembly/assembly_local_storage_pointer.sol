contract C {
	uint256[] public a;

	function f() public {
		require(a.length == 0);
		uint256[] storage x = a;
		assert(x.length == 0); // should hold
		uint256 i = 7;
		assembly {
			sstore(x.slot, 7)
		}
		assert(x.length == 0); // should fail
		assert(x.length == 7); // should hold, but the analysis cannot know this yet
		assert(i == 7); // should hold, not changed by the assembly
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 7737: (170-205): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 6328: (208-229): CHC: Assertion violation happens here.
// Warning 6328: (248-269): CHC: Assertion violation happens here.

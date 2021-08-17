contract C {
	uint256 public z;

	function g() public {
		z = 42;
		uint i = 32;
		assembly {
			function f() {
				sstore(z.slot, 7)
			}
			f()
		}
		assert(z == 42); // should fail
		assert(z == 7); // should hold, but the analysis cannot know this yet
		assert(i == 32); // should hold, not changed by the assembly
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 7737: (83-149): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 6328: (152-167): CHC: Assertion violation happens here.
// Warning 6328: (186-200): CHC: Assertion violation happens here.
// Warning 7737: (83-149): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).

pragma experimental SMTChecker;

contract C {
	uint256 public z;

	function f() public {
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
// ----
// Warning 7737: (116-182): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 6328: (185-200): CHC: Assertion violation happens here.\nCounterexample:\nz = 0\ni = 32\n\nTransaction trace:\nC.constructor()\nState: z = 0\nC.f()
// Warning 6328: (219-233): CHC: Assertion violation happens here.\nCounterexample:\nz = 43\ni = 32\n\nTransaction trace:\nC.constructor()\nState: z = 0\nC.f()
// Warning 7737: (116-182): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).

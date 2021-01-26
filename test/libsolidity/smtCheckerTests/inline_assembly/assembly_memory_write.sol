pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
	}

	S s;

	function f() public {
		s.x = 42;
		S memory sm = s;
		assert(sm.x == 42); // should hold
		uint256 i = 7;
		assembly {
			mstore(sm, i)
		}
		assert(sm.x == 42); // should fail
		assert(sm.x == 7); // should hold, but the analysis cannot know this yet
		assert(s.x == 42); // should hold, storage not changed by the assembly
		assert(i == 7); // should hold, not changed by the assembly
	}
}
// ----
// Warning 7737: (189-220): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 6328: (223-241): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 42}\n\nTransaction trace:\nC.constructor()\nState: s = {x: 0}\nC.f()
// Warning 6328: (260-277): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 42}\n\nTransaction trace:\nC.constructor()\nState: s = {x: 0}\nC.f()
// Warning 7737: (189-220): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).

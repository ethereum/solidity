contract C {
	struct S {
		uint x;
	}

	S s;

	function f() public {
		s.x = 42;
		S storage sm = s;
		assert(sm.x == 42); // should hold
		uint256 i = 7;
		assembly {
			sstore(sm.slot, i)
		}
		sm.x = 10;
		assert(sm.x == 10); // should hold
		assert(i == 7); // should hold, not changed by the assembly
	}
}
// ====
// SMTEngine: all
// ----
// Warning 7737: (157-193): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

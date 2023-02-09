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
		sm.x = 10;
		assert(sm.x == 10); // should hold
		assert(s.x == 42); // should hold, storage not changed by the assembly
		assert(i == 7); // should hold, not changed by the assembly
	}
}
// ====
// SMTEngine: all
// ----
// Warning 7737: (156-187): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

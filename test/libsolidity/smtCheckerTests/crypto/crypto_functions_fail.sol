contract C {
	function k(bytes memory b0, bytes memory b1) public pure {
		bytes32 k0 = keccak256(b0);
		bytes32 k1 = keccak256(b1);
		assert(k0 == k1);
	}
	function s(bytes memory b0, bytes memory b1) public pure {
		bytes32 s0 = sha256(b0);
		bytes32 s1 = sha256(b1);
		// Disabled because of Spacer nondeterminism.
		//assert(s0 == s1);
	}
	function r(bytes memory b0, bytes memory b1) public pure {
		bytes32 r0 = ripemd160(b0);
		bytes32 r1 = ripemd160(b1);
		// Disabled because of Spacer nondeterminism.
		//assert(r0 == r1);
	}
	function e(bytes32 h0, uint8 v0, bytes32 r0, bytes32 s0, bytes32 h1, uint8 v1, bytes32 r1, bytes32 s1) public pure {
		address a0 = ecrecover(h0, v0, r0, s0);
		address a1 = ecrecover(h1, v1, r1, s1);
		// Disabled because of Spacer nondeterminism.
		//assert(a0 == a1);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (218-228): Unused local variable.
// Warning 2072: (245-255): Unused local variable.
// Warning 2072: (405-415): Unused local variable.
// Warning 2072: (435-445): Unused local variable.
// Warning 2072: (656-666): Unused local variable.
// Warning 2072: (698-708): Unused local variable.
// Warning 1218: (135-151): CHC: Error trying to invoke SMT solver.
// Warning 6328: (135-151): CHC: Assertion violation might happen here.
// Warning 4661: (135-151): BMC: Assertion violation happens here.

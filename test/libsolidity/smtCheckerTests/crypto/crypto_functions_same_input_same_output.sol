contract C {
	function k(bytes memory b0) public pure {
		bytes memory b1 = b0;
		bytes32 k0 = keccak256(b0);
		bytes32 k1 = keccak256(b1);
		assert(k0 == k1);
	}
	function s(bytes memory b0) public pure {
		bytes memory b1 = b0;
		bytes32 s0 = sha256(b0);
		bytes32 s1 = sha256(b1);
		assert(s0 == s1);
	}
	function r(bytes memory b0) public pure {
		bytes memory b1 = b0;
		bytes32 r0 = ripemd160(b0);
		bytes32 r1 = ripemd160(b1);
		assert(r0 == r1);
	}
	function e(bytes32 h0, uint8 v0, bytes32 r0, bytes32 s0) public pure {
		(bytes32 h1, uint8 v1, bytes32 r1, bytes32 s1) = (h0, v0, r0, s0);
		address a0 = ecrecover(h0, v0, r0, s0);
		address a1 = ecrecover(h1, v1, r1, s1);
		assert(a0 == a1);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

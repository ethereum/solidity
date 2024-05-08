contract C {
	function c1() public pure {
		bytes32 k1 = keccak256(abi.encode([1]));
		bytes32 k2 = keccak256(abi.encode([1]));
		assert(k1 == k2);
	}

        function c2() public pure {
		bytes32 s1 = sha256(abi.encode([1,2]));
		bytes32 s2 = sha256(abi.encode([1,2]));
		assert(s1 == s2);
	}

        function c3() public pure {
		bytes32 r1 = ripemd160(abi.encode([1,2,3]));
		bytes32 r2 = ripemd160(abi.encode([1,2,3]));
		assert(r1 == r2);
	}
}
// ====
// SMTEngine: chc
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

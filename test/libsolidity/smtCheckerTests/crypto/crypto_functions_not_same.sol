contract C {
	function f(bytes memory data) public pure {
		bytes32 k = keccak256(data);
		fi(data, k);
	}
	function fi(bytes memory data, bytes32 k) internal pure {
		bytes32 h = sha256(data);
		assert(h == k);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (196-210): CHC: Assertion violation happens here.

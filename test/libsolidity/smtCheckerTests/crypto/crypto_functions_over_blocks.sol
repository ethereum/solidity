pragma experimental SMTChecker;

contract C {
	function f(bytes memory data) public pure {
		bytes32 k = keccak256(data);
		fi(data, k);
	}
	function fi(bytes memory data, bytes32 k) internal pure {
		bytes32 h = keccak256(data);
		assert(h == k);
	}
}

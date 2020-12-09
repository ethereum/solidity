pragma experimental SMTChecker;

contract C {
	function f(bytes memory data) public pure {
		bytes32 k = keccak256(data);
		bytes32 s = sha256(data);
		bytes32 r = ripemd160(data);
		assert(k == s);
		assert(s == r);
		assert(r == k);
	}
}
// ----
// Warning 6328: (183-197): CHC: Assertion violation happens here.\nCounterexample:\n\ndata = [7, 7]\n\n\nTransaction trace:\nconstructor()\nf([7, 7])
// Warning 6328: (201-215): CHC: Assertion violation happens here.\nCounterexample:\n\ndata = [9, 9]\n\n\nTransaction trace:\nconstructor()\nf([9, 9])
// Warning 6328: (219-233): CHC: Assertion violation happens here.\nCounterexample:\n\ndata = [7, 7]\n\n\nTransaction trace:\nconstructor()\nf([7, 7])

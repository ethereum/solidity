contract C {
	function f(string memory sig, uint x, uint[] memory a) public pure {
		bytes memory b1 = abi.encodeWithSignature(sig, x, a);
		bytes memory b2 = abi.encodeWithSelector(bytes4(keccak256(bytes(sig))), x, a);
		// should hold but we do not evaluate keccak256 in an interpreted way
		//assert(b1.length == b2.length);
		assert(b1.length != b2.length); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (330-360): CHC: Assertion violation might happen here.
// Warning 7812: (330-360): BMC: Assertion violation might happen here.

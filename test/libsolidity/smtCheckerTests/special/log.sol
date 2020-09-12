pragma experimental SMTChecker;

contract C {
	function f() external {
		bytes32 t1 = bytes32(uint256(0x1234));
		log0(t1);
		log1(t1, t1);
		log2(t1, t1, t1);
		log3(t1, t1, t1, t1);
		log4(t1, t1, t1, t1, t1);
	}
	function g_data() pure internal returns (bytes32) {
		assert(true);
		return bytes32(uint256(0x5678));
	}
	function g() external {
		// To test that the function call is actually visited.
		log0(g_data());
		log1(g_data(), g_data());
		log2(g_data(), g_data(), g_data());
		log3(g_data(), g_data(), g_data(), g_data());
		log4(g_data(), g_data(), g_data(), g_data(), g_data());
	}
	bool x = true;
	function h_data() view internal returns (bytes32) {
		assert(x);
	}
	function h() external {
		// To test that the function call is actually visited.
		x = false;
		log0(h_data());
		log1(h_data(), h_data());
		log2(h_data(), h_data(), h_data());
		log3(h_data(), h_data(), h_data(), h_data());
		log4(h_data(), h_data(), h_data(), h_data(), h_data());
	}
}
// ----
// Warning 6328: (668-677): Assertion violation happens here.

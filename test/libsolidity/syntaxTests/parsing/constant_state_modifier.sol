contract C {
	uint s;
	// this test should fail starting from 0.5.0
	function f() public constant returns (uint) {
		return s;
	}
}

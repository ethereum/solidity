contract C {
	function f(bytes calldata bCalldata) public pure returns (uint, bytes32, bytes32, uint256) {
		bytes memory bMemory;
		bytes32 b;
		bytes32 c;
		uint x;
		((bMemory, b, c), x) = ((bCalldata, "abc", "def"), 42);
		return (bMemory.length, b, c, x);
	}
}
// ----
// f(bytes): 0x20, 3, "abc" -> 3, 0x6162630000000000000000000000000000000000000000000000000000000000, 0x6465660000000000000000000000000000000000000000000000000000000000, 0x2a

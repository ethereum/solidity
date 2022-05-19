// Triggered ICE before
contract C {
	function f(string calldata data) external pure returns(string memory) {
		bytes calldata test = bytes(data[:3]);
		return string(test);
	}
}
// ----
// f(string): 0x20, 3, "123" -> 0x20, 3, "123"

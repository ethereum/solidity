contract C {
	function f(string memory s) public pure returns (bytes memory t) {
		t = bytes(s);
	}
}
// ----
// f(string): 32, 5, "Hello" -> 32, 5, "Hello"

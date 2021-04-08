contract C {
	function f() public pure {
		string memory x = "Hello World";
		assert(bytes(x).length == 11);
	}
	function g() public pure {
		string memory x = unicode"Hello World";
		assert(bytes(x).length == 11);
	}
	function h() public pure {
		bytes memory x = unicode"Hello World";
		string memory y = string(x);
		assert(bytes(y).length == 11);
	}
}
// ====
// SMTEngine: all
// ----

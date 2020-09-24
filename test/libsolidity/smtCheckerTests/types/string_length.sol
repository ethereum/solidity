pragma experimental SMTChecker;

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
// ----
// Warning 6328: (111-140): Assertion violation happens here.
// Warning 6328: (217-246): Assertion violation happens here.
// Warning 6328: (353-382): Assertion violation happens here.

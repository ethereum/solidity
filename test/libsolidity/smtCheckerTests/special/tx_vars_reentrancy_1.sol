interface I {
	function f() external;
}

contract C {
	function g(I _i) public payable {
		uint x = address(this).balance;
		_i.f();
		assert(x == address(this).balance); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTIgnoreOS: macos
// ----
// Warning 6328: (135-169): CHC: Assertion violation happens here.

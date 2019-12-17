contract C {
	function g(address payable _p) internal pure returns (uint) {
		return 1;
	}
	function f(address _a) public pure {
		uint x = g(payable(_a));
		uint y = g(_a);
	}
}
// ----
// TypeError: (169-171): Invalid type for argument in function call. Invalid implicit conversion from address to address payable requested.

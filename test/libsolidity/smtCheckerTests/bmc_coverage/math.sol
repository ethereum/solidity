pragma experimental SMTChecker;
contract C {
	uint z = 1;
	uint w = z - 3;
	function a(uint x, uint y) public pure returns (uint) {
		return x + y;
	}
	function s(uint x, uint y) public pure returns (uint) {
		return x - y;
	}
	function m(uint x, uint y) public pure returns (uint) {
		return x * y;
	}
	function d(uint x, uint y) public pure returns (uint) {
		return x / y;
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 2661: (141-146): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4144: (217-222): BMC: Underflow (resulting value less than 0) happens here.
// Warning 2661: (293-298): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 3046: (369-374): BMC: Division by zero happens here.
// Warning 4144: (68-73): BMC: Underflow (resulting value less than 0) happens here.

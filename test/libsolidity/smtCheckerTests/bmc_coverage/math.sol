pragma experimental SMTChecker;
contract C {
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
// Warning 2661: (111-116): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4144: (187-192): BMC: Underflow (resulting value less than 0) happens here.
// Warning 2661: (263-268): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 3046: (339-344): BMC: Division by zero happens here.

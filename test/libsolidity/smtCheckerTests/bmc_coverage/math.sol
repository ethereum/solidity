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
// Warning 2661: (79-84): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4144: (155-160): BMC: Underflow (resulting value less than 0) happens here.
// Warning 2661: (231-236): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 3046: (307-312): BMC: Division by zero happens here.

pragma experimental SMTChecker;
contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
	}
	bool b = (f() > 0) || (f() > 0);
}
// ----
// Warning 2661: (100-105): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning 4144: (100-105): Underflow (resulting value less than 0) happens here
// Warning 2661: (100-105): Overflow (resulting value larger than 2**256 - 1) happens here

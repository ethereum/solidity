pragma experimental SMTChecker;

contract C
{
	function f(address a, address b) public view {
		uint x = b.balance + 1000 ether;
		assert(a.balance > b.balance);
	}
}
// ----
// Warning 2072: (96-102): Unused local variable.
// Warning 4984: (105-127): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\na = 0\nb = 0\n\n\nTransaction trace:\nconstructor()\nf(0, 0)
// Warning 6328: (131-160): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nb = 0\n\n\nTransaction trace:\nconstructor()\nf(0, 0)

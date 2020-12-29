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
// Warning 4984: (105-127): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\na = 0\nb = 38\n\n\nTransaction trace:\nC.constructor()\nC.f(0, 38)
// Warning 6328: (131-160): CHC: Assertion violation happens here.\nCounterexample:\n\na = 7719\nb = 38\n\n\nTransaction trace:\nC.constructor()\nC.f(7719, 38)

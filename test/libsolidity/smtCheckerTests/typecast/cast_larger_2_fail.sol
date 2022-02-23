contract C
{
	function f() public pure {
		uint16 a = 0x1234;
		uint32 b = uint32(a); // b will be 0x00001234 now
		assert(a != b);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (116-130): CHC: Assertion violation happens here.\nCounterexample:\n\na = 4660\nb = 4660\n\nTransaction trace:\nC.constructor()\nC.f()

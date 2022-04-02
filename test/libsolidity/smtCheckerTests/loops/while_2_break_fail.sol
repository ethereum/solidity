contract C
{
	function f(uint x) public pure {
		while (x == 0) {
			++x;
			break;
			++x;
		}
		assert(x == 2);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTSolvers: z3
// ----
// Warning 5740: (87-90='++x'): Unreachable code.
// Warning 6328: (98-112='assert(x == 2)'): CHC: Assertion violation happens here.

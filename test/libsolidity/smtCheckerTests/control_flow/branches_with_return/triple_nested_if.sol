contract C {

    uint a;
    uint b;
    uint c;

	function test() public view {
		if (a == 0) {
			if (b == 0) {
				if (c == 0) {
					return;
				}
			}
		}
		assert(a != 0 || b != 0 || c != 0);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Info 1180: Contract invariant(s) for :C:\n((c <= 0) && (a <= 0) && (b <= 0))\n

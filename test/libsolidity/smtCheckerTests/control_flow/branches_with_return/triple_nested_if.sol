pragma experimental SMTChecker;

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

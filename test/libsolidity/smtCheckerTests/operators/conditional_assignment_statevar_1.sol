pragma experimental SMTChecker;

contract C {
    uint a;
    bool b;

	constructor(bool _b) {
		b = _b;
	}

    function f() public returns(uint c) {
        c = b ? a + 10 : ++a;
        assert(c >= a);
    }
}
// ----
// Warning 4984: (167-173): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (176-179): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 2661: (167-173): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (176-179): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.

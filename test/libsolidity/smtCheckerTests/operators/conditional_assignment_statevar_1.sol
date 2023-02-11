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
// ====
// SMTEngine: all
// ----
// Warning 4984: (143-146): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Warning 2661: (143-146): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.

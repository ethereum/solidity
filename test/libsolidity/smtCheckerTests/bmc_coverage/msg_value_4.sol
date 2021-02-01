pragma experimental SMTChecker;
contract A {
    uint x = msg.value;
    constructor() {
        assert(x == 0); // should hold
    }
}

contract B {
    constructor() payable {
        assert(msg.value == 0); // should fail
    }
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (186-208): BMC: Assertion violation happens here.

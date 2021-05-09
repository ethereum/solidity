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
// SMTEngine: all
// ----
// Warning 6328: (154-176): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nB.constructor(){ value: 1 }

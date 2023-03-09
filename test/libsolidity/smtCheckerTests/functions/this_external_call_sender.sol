contract C {
    address lastCaller;

    constructor() {
        lastCaller = msg.sender;
    }

    modifier log {
        lastCaller = msg.sender;
        _;
    }

    function test() log public {
        assert(lastCaller == msg.sender);
        this.g();
        assert(lastCaller == address(this));
        assert(lastCaller == msg.sender);
        assert(lastCaller == address(0));
    }

    function g() log public {
    }
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (314-346): CHC: Assertion violation happens here.
// Warning 6328: (356-388): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

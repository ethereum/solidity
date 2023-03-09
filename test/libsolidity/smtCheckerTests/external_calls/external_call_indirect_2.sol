contract A {
    uint x;
    address immutable owner;
    constructor() {
        owner = msg.sender;
    }
    function setX(uint _x) public {
        require(msg.sender == owner);
        x = _x;
    }
    function getX() public view returns (uint) {
        return x;
    }
}

contract B {
    A a;
    constructor() {
        a = new A();
        assert(a.getX() == 0); // should hold
    }
    function g() public view {
        assert(a.getX() == 0); // should hold, but fails because
        // the nondet_interface constraint added for `A a` in between
        // txs of `B` does not have the constraint that `msg.sender != address(this)`
        // so `A.setX` is allowed with `msg.sender = address(this)` inside
        // the current rules defining nondet_interface.
        // If we want to support that, we likely need a new type of nondet_interface
        // `nondet_interface_with_tx` that contains tx data as well as restricts
        // every further `nondet_interface_with_tx` to not have that `msg.sender`.
    }
    function getX() public view returns (uint) {
        return a.getX();
    }
}

contract C {
    B b;
    constructor() {
        b = new B();
        assert(b.getX() == 0); // should hold
    }
    function f() public view {
        assert(b.getX() == 0); // should hold
    }
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// ----
// Warning 6328: (434-455): CHC: Assertion violation happens here.
// Warning 6328: (1270-1291): CHC: Assertion violation might happen here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

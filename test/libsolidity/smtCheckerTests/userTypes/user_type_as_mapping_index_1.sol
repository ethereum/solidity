contract C {
    type T is bool;
    mapping (T => uint) s;

    constructor() {
        s[C.T.wrap(true)] = 42;
        s[C.T.wrap(false)] = 2;
    }

    function f(bool b) external view {
        assert(s[C.T.wrap(b)] > 0); // should hold
    }

    function g(T b) external view {
        require(C.T.unwrap(b));
        assert(s[b] == 2); // should fail
    }
}
// ----
// Warning 6328: (325-342): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

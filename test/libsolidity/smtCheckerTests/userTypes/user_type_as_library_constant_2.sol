type T is bool;
library L{
    T constant c = T.wrap(false);
    uint constant z = 43;
}
T constant g = T.wrap(false);

contract C {
    T constant b = T.wrap(false);
    uint constant X = 43;

    function f() external pure {
        T x = T.wrap(false);
        assert(T.unwrap(x)); // should fail

        assert(L.z == 42); // should fail
        assert(T.unwrap(L.c)); // should fail

        assert(T.unwrap(g)); // should fail

        assert(C.X == 42); // should fail
        assert(T.unwrap(b)); // should fail
        assert(T.unwrap(C.b)); // should fail
    }
}
// ----
// Warning 6328: (264-283): CHC: Assertion violation happens here.\nCounterexample:\nX = 43\n\nTransaction trace:\nC.constructor()\nState: X = 43\nC.f()
// Warning 6328: (309-326): CHC: Assertion violation happens here.\nCounterexample:\nX = 43\n\nTransaction trace:\nC.constructor()\nState: X = 43\nC.f()
// Warning 6328: (351-372): CHC: Assertion violation happens here.\nCounterexample:\nX = 43\n\nTransaction trace:\nC.constructor()\nState: X = 43\nC.f()
// Warning 6328: (398-417): CHC: Assertion violation happens here.\nCounterexample:\nX = 43\n\nTransaction trace:\nC.constructor()\nState: X = 43\nC.f()
// Warning 6328: (443-460): CHC: Assertion violation happens here.\nCounterexample:\nX = 43\n\nTransaction trace:\nC.constructor()\nState: X = 43\nC.f()
// Warning 6328: (485-504): CHC: Assertion violation happens here.\nCounterexample:\nX = 43\n\nTransaction trace:\nC.constructor()\nState: X = 43\nC.f()
// Warning 6328: (529-550): CHC: Assertion violation happens here.\nCounterexample:\nX = 43\n\nTransaction trace:\nC.constructor()\nState: X = 43\nC.f()

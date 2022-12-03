contract C {
    uint x = ((((C)))).x = ((C)).x = 2;

    function f() external view {
        assert(x == 2); // should fail for D
    }

    function g() external view {
        assert(x != 2); // should fail for C
    }
}

contract D is C {
    uint y = ((C)).x = 3;

    function h() external view {
        assert(y == 3); // should hold
    }
}
// ----
// Warning 6328: (95-109): CHC: Assertion violation happens here.\nCounterexample:\ny = 3, x = 3\n\nTransaction trace:\nD.constructor()\nState: y = 3, x = 3\nC.f()
// Warning 6328: (180-194): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\nTransaction trace:\nC.constructor()\nState: x = 2\nC.g()

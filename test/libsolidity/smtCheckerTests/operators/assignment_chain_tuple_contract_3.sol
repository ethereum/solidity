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
// Warning 6328: (95-109): CHC: Assertion violation happens here.
// Warning 6328: (180-194): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.

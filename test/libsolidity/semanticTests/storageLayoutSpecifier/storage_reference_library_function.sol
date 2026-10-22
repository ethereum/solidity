pragma abicoder v2;

struct S {
    uint x;
    bool b;
}

library L {
    function validate(S memory ptr) public {
        require(ptr.x == 2);
        require(ptr.b);
    }
}

contract A {
    S public s;
}

contract C is A layout at 42 {
    function initUsingReference() public {
        S storage ptr = s;
        ptr.x = 2;
        ptr.b = true;

        L.validate(s);
    }
}
// ====
// targetContract: C
// ----
// library: L
// initUsingReference() ->
// s() -> 2, true

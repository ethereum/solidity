pragma abicoder v2;
contract A {
    struct S {
        uint x;
        bool b;
    }
    S public s;
}

contract C is A layout at 42 {
    function InitUsingReference() public {
        S storage ptr = s;
        ptr.x = 2;
        ptr.b = true;

        validate(s);
    }
    function validate(S memory ptr) public pure {
        require(ptr.x == 2);
        require(ptr.b);
    }
}
// ====
// targetContract: C
// ----
// InitUsingReference() ->
// s() -> 2, true

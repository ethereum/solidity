struct S { uint a; }
contract C {
    function f() public pure {
        S memory s = S(42);
        s;
    }
}
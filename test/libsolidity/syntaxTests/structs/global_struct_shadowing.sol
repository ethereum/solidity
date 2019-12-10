struct S { uint a; }
contract C {
    struct S { address x; }
    function f() public view {
        S memory s = S(address(this));
        s;
    }
}
// ----
// Warning: (38-61): This declaration shadows an existing declaration.

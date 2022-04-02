struct S { uint a; }
contract C {
    struct S { address x; }
    function f() public view {
        S memory s = S(address(this));
        s;
    }
}
// ----
// Warning 2519: (38-61='struct S { address x; }'): This declaration shadows an existing declaration.

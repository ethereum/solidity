pragma experimental "v0.5.0";
contract C {
    struct s {
        uint a;
    }
    function f() public {
        s storage x;
    }
}
// ----
// DeclarationError: (114-125): Uninitialized storage pointer.

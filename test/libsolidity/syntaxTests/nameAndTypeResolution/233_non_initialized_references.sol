contract C {
    struct s {
        uint a;
    }
    function f() public {
        s storage x;
        x.a = 2;
    }
}
// ----
// DeclarationError: (84-95): Uninitialized storage pointer.

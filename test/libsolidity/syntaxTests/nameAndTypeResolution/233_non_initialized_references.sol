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
// TypeError 3464: (105-106): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.

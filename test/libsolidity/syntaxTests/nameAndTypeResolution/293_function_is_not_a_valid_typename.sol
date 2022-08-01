contract test {
    function foo() public {
    }

    function f() public {
        foo g;
    }
}
// ----
// TypeError 5172: (85-88): Name has to refer to a user-defined value type, struct, enum or contract.

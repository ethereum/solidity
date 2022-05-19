contract C {
    function foo() internal returns (uint) {
        return 42;
    }

    function get_ptr(function() internal returns (uint) ptr) internal returns(function() internal returns (uint)) {
        return ptr;
    }

    function associated() public returns (uint) {
        // This expression directly references function definition
        return (foo)();
    }

    function unassociated() public returns (uint) {
        // This expression is not associated with a specific function definition
        return (get_ptr(foo))();
    }
}
// ====
// compileToEwasm: also
// ----
// associated() -> 42
// unassociated() -> 42

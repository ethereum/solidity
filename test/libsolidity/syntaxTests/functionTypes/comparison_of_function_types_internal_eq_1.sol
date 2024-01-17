contract C {
    function f() public pure returns (bool ret) {
        return f == f;
    }
    function g() public pure returns (bool ret) {
        return f != f;
    }
}
// ----
// Warning 3075: (78-84): Comparison of internal function pointers can yield unexpected results in the legacy pipeline with the optimizer enabled, and will be disallowed entirely in the next breaking release.
// Warning 3075: (157-163): Comparison of internal function pointers can yield unexpected results in the legacy pipeline with the optimizer enabled, and will be disallowed entirely in the next breaking release.

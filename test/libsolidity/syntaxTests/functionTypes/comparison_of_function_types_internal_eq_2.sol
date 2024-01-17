contract C {
    function f() internal {}
    function g() internal {}

    function test() public pure returns (bool) {
        function () internal ptr = C.f;
        return ptr == C.g;
    }
}
// ----
// Warning 3075: (176-186): Comparison of internal function pointers can yield unexpected results in the legacy pipeline with the optimizer enabled, and will be disallowed entirely in the next breaking release.

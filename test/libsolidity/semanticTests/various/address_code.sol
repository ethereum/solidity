contract C {
    bytes public initCode;

    constructor() {
        // This should catch problems, but lets also test the case the optimiser is buggy.
        assert(address(this).code.length == 0);
        initCode = address(this).code;
    }

    // To avoid dependency on exact length.
    function f() public view returns (bool) { return address(this).code.length > 380; }
    function g() public view returns (uint) { return address(0).code.length; }
    function h() public view returns (uint) { return address(1).code.length; }
}
// ----
// constructor() ->
// gas irOptimized: 71218
// gas irOptimized code: 100400
// gas legacy: 82796
// gas legacy code: 159000
// gas legacyOptimized: 69758
// gas legacyOptimized code: 83600
// initCode() -> 0x20, 0
// f() -> true
// g() -> 0
// h() -> 0

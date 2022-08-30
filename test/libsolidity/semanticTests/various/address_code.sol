contract C {
    bytes public initCode;

    constructor() {
        // This should catch problems, but lets also test the case the optimiser is buggy.
        assert(address(this).code.length == 0);
        initCode = address(this).code;
    }

    // To avoid dependency on exact length.
    function f() public view returns (bool) { return address(this).code.length > 400; }
    function g() public view returns (uint) { return address(0).code.length; }
    function h() public view returns (uint) { return address(1).code.length; }
}
// ====
// compileToEwasm: also
// ----
// constructor() ->
// gas irOptimized: 175791
// gas legacy: 247263
// gas legacyOptimized: 155977
// initCode() -> 0x20, 0
// f() -> true
// g() -> 0
// h() -> 0

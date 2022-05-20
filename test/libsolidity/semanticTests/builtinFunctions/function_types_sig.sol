contract C {
    uint256 public x;

    function f() public pure returns (bytes4) {
        return this.f.selector;
    }

    function g() public returns (bytes4) {
        function () pure external returns (bytes4) fun = this.f;
        return fun.selector;
    }

    function h() public returns (bytes4) {
        function () pure external returns (bytes4) fun = this.f;
        return fun.selector;
    }

    function i() public pure returns (bytes4) {
        return this.x.selector;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 0x26121ff000000000000000000000000000000000000000000000000000000000
// g() -> 0x26121ff000000000000000000000000000000000000000000000000000000000
// h() -> 0x26121ff000000000000000000000000000000000000000000000000000000000
// i() -> 0x0c55699c00000000000000000000000000000000000000000000000000000000

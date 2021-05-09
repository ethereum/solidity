contract C {
    function f() public pure returns (string memory) {
        return "";
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 0x20, 0

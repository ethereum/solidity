// This caused a failure since the type was not converted to its mobile type.
contract C {
    function f() public returns (uint256) {
        return [4][0];
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 4

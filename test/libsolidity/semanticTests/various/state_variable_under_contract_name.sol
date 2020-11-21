contract Scope {
    uint256 stateVar = 42;

    function getStateVar() public view returns (uint256 stateVar) {
        stateVar = Scope.stateVar;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// getStateVar() -> 42

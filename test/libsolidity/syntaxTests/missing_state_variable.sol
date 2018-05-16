contract Scope {
    function getStateVar() view public returns (uint stateVar) {
        stateVar = Scope.stateVar; // should fail.
    }
}
// ----
// TypeError: (101-115): Member "stateVar" not found or not visible after argument-dependent lookup in type(contract Scope)

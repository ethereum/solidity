contract Test {
    function f() public pure returns (bytes memory) {
        type(Test);
    }
}
// ----
// Warning 6321: (54-66): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6133: (78-88): Statement has no effect.

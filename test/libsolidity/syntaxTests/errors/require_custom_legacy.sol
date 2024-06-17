error E(uint a, uint b);
contract C {
    function f(bool c) public pure {
        require(c, E(2, 7));
    }
}
// ----
// UnimplementedFeatureError 1834: Require with a custom error is only available using the via-ir pipeline.

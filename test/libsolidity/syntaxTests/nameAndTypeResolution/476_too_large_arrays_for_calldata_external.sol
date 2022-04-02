contract C {
    function f(uint[85678901234] calldata a) pure external {
    }
}
// ----
// TypeError 1534: (28-56='uint[85678901234] calldata a'): Type too large for calldata.

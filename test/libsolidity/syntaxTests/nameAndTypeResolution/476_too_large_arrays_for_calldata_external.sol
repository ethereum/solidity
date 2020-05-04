contract C {
    function f(uint[85678901234] calldata a) pure external {
    }
}
// ----
// TypeError: (28-56): Type too large for calldata.

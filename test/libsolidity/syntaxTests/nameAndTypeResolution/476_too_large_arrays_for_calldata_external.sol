contract C {
    function f(uint[85678901234] calldata a) pure external {
    }
}
// ----
// TypeError: (28-56): Array is too large to be encoded.

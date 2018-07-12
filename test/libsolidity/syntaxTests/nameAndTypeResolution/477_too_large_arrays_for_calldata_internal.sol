contract C {
    function f(uint[85678901234] memory a) pure internal {
    }
}
// ----
// TypeError: (28-54): Array is too large to be encoded.

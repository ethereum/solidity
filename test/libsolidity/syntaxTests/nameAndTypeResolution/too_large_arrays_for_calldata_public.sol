contract C {
    function f(uint[85678901234] a) pure public {
    }
}
// ----
// TypeError: (28-47): Array is too large to be encoded.

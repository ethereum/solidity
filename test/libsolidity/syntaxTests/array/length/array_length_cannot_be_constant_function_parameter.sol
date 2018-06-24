contract C {
    function f(uint constant LEN) {
        uint[LEN] a;
    }
}
// ----
// TypeError: (62-65): Invalid array length, expected integer literal or constant expression.

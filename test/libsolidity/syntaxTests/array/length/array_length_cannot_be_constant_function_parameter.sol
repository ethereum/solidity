contract C {
    function f(uint constant LEN) public {
        uint[LEN] a;
    }
}
// ----
// TypeError: (69-72): Invalid array length, expected integer literal or constant expression.

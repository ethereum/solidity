contract C {
    function f(bool c) public pure {
        if (c) {
            return;
        } else {
            return;
        }
        return; // unreachable
    }
}
// ----
// Warning: (142-149): Unreachable code.

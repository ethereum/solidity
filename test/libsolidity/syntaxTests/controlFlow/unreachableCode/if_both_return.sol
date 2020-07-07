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
// Warning 5740: (142-149): Unreachable code.

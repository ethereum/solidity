contract C {
    function f() public pure {
        for (uint a = 0; a < 1; a++) {
            break;
            uint b = 42; b;
        }
        return;
    }
}
// ----
// Warning 5740: (76-79='a++'): Unreachable code.
// Warning 5740: (114-128='uint b = 42; b'): Unreachable code.

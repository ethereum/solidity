contract C {
    function f() public pure {
        do {
            uint a = 42; a;
            continue;
            return; // this is unreachable
        } while(false);
        return; // this is still reachable
    }
}
// ----
// Warning: (119-126): Unreachable code.

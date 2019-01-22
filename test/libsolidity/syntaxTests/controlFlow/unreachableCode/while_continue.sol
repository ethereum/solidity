contract C {
    function f() public pure {
        while(true) {
            continue;
            return;
        }
        return; // this is unreachable as well, but currently undetected (needs to consider constant condition "true")
    }
}
// ----
// Warning: (100-107): Unreachable code.

contract C {
    function f() public pure {
        var (b, c);
        b.WeMustNotReachHere();
        c.FailsToLookupToo();
    }
}
// ----
// TypeError: (52-62): Use of the "var" keyword is disallowed.

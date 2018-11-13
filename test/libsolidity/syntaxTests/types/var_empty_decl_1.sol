contract C {
    function f() public pure {
        var a;
        a.NeverReachedByParser();
    }
}
// ----
// TypeError: (52-57): Use of the "var" keyword is disallowed.

contract C {
    function foo() pure public returns (int) {
        for (42; true;) {
            int i = 42;
            return i + 1;
        }
        return 0;
    }
}
// ----
// Warning 6133: (73-75): Statement has no effect.

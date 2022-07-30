contract C {
    function foo() pure public returns (int) {
        for (; true;) {
            int i = 42;
            return i + 1;
        }
        return 0;
    }
}
// ----

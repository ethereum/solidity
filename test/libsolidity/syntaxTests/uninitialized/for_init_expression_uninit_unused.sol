contract C {
    function foo() pure public returns (int) {
        for (int i; true;) {
            int i = i;
            return i + 1;
        }
        return 0;
    }
}
// ----
// Warning 2519: (101-106): This declaration shadows an existing declaration.
// Warning 4716: (73-78): Uninitialized variable in for-loop initialization expression.

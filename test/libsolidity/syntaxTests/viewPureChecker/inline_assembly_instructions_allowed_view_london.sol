contract C {
    function f() public view {
        assembly {
            // Renamed in paris
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: =london
// ----
// Warning 5740: (94-1733): Unreachable code.
// Warning 5740: (1746-1758): Unreachable code.
// Warning 5740: (1801-1810): Unreachable code.
// Warning 5740: (1978-2244): Unreachable code.

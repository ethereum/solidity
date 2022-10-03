contract C {
    function f() public {
        assembly {
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: =london
// ----
// Warning 5740: (89-1716): Unreachable code.
// Warning 5740: (1729-1741): Unreachable code.
// Warning 5740: (1754-1769): Unreachable code.
// Warning 5740: (1782-1791): Unreachable code.
// Warning 5740: (1804-2215): Unreachable code.

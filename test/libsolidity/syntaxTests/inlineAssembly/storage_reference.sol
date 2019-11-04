contract C {
    uint[] x;
    fallback() external {
        uint[] storage y = x;
        assembly {
            pop(y)
        }
    }
}
// ----
// TypeError: (118-119): You have to use the _slot or _offset suffix to access storage reference variables.

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
// TypeError 9068: (118-119): You have to use the .slot or .offset suffix to access storage reference variables.

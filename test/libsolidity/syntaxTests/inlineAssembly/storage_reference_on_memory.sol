contract C {
    uint[] x;
    fallback() external {
        uint[] memory y = x;
        assembly {
            pop(y.slot)
            pop(y.offset)
        }
    }
}
// ----
// TypeError 3622: (117-123): The suffix ".slot" is not supported by this variable or type.
// TypeError 3622: (141-149): The suffix ".offset" is not supported by this variable or type.

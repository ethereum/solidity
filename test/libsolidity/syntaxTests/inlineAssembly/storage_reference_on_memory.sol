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
// TypeError 3622: (117-123): The suffixes .offset and .slot can only be used on storage variables.
// TypeError 3622: (141-149): The suffixes .offset and .slot can only be used on storage variables.

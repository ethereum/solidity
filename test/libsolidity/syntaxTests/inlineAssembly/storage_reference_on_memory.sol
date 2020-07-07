contract C {
    uint[] x;
    fallback() external {
        uint[] memory y = x;
        assembly {
            pop(y_slot)
            pop(y_offset)
        }
    }
}
// ----
// TypeError 3622: (117-123): The suffixes _offset and _slot can only be used on storage variables.
// TypeError 3622: (141-149): The suffixes _offset and _slot can only be used on storage variables.

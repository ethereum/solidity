contract C {
    uint[] x;
    fallback() external {
        uint[] storage y = x;
        assembly {
            pop(y_slot)
            pop(y_offset)
        }
    }
}
// ----
// DeclarationError 9467: (118-124): Identifier not found. Use ".slot" and ".offset" to access storage variables.
// DeclarationError 9467: (142-150): Identifier not found. Use ".slot" and ".offset" to access storage variables.

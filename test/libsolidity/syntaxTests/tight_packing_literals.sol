contract C {
    function k() pure public returns (bytes memory) {
        return abi.encodePacked(1);
    }
}

// ----
// TypeError: (99-100): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.

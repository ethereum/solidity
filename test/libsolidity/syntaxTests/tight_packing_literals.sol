contract C {
    function k() pure public returns (bytes) {
        return abi.encodePacked(1);
    }
}

// ----
// TypeError: (92-93): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.

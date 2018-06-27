pragma experimental "v0.5.0";
contract C {
    function k() pure public returns (bytes) {
        return abi.encodePacked(1);
    }
}

// ----
// TypeError: (122-123): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.

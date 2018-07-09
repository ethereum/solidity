pragma experimental "v0.5.0";
contract C {
    function k() pure public returns (bytes memory) {
        return abi.encodePacked(1);
    }
}

// ----
// TypeError: (129-130): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.

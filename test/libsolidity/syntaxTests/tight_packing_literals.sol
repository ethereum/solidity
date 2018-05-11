contract C {
    function k() pure public returns (bytes memory) {
        return abi.encodePacked(1);
    }
}

// ----
// Warning: (99-100): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.

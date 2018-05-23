contract C {
    function k() pure public returns (bytes) {
        return abi.encodePacked(1);
    }
}

// ----
// Warning: (92-93): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.

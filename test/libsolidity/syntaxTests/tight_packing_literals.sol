contract C {
    function f() pure public returns (bytes32) {
        return keccak256(1);
    }
    function h() pure public returns (bytes32) {
        return sha256(1);
    }
    function j() pure public returns (bytes32) {
        return ripemd160(1);
    }
    function k() pure public returns (bytes) {
        return abi.encodePacked(1);
    }
}

// ----
// Warning: (87-88): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (77-89): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (77-89): The provided argument of type int_const 1 is not implicitly convertible to expected type bytes memory.
// Warning: (168-169): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (161-170): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (161-170): The provided argument of type int_const 1 is not implicitly convertible to expected type bytes memory.
// Warning: (252-253): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (242-254): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (242-254): The provided argument of type int_const 1 is not implicitly convertible to expected type bytes memory.
// Warning: (341-342): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.

contract C {
    function f() pure public returns (bytes32) {
        return keccak256(1);
    }
    function g() pure public returns (bytes32) {
        return sha3(1);
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
// Warning: (161-168): "sha3" has been deprecated in favour of "keccak256"
// Warning: (166-167): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (161-168): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (161-168): The provided argument of type int_const 1 is not implicitly convertible to expected type bytes memory.
// Warning: (247-248): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (240-249): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (240-249): The provided argument of type int_const 1 is not implicitly convertible to expected type bytes memory.
// Warning: (331-332): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.
// Warning: (321-333): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (321-333): The provided argument of type int_const 1 is not implicitly convertible to expected type bytes memory.
// Warning: (420-421): The type of "int_const 1" was inferred as uint8. This is probably not desired. Use an explicit type to silence this warning.

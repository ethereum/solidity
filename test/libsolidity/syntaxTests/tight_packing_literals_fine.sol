contract C {
    function f() pure public returns (bytes32) {
        return keccak256(uint8(1));
    }
    function h() pure public returns (bytes32) {
        return sha256(uint8(1));
    }
    function j() pure public returns (bytes32) {
        return ripemd160(uint8(1));
    }
    function k() pure public returns (bytes) {
        return abi.encodePacked(uint8(1));
    }
    function l() pure public returns (bytes) {
        return abi.encode(1);
    }
}
// ----
// Warning: (77-96): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (77-96): The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.
// Warning: (168-184): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (168-184): The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.
// Warning: (256-275): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (256-275): The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.

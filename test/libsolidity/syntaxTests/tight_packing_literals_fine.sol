contract C {
    function f() pure public returns (bytes32) {
        return keccak256(uint8(1));
    }
    function g() pure public returns (bytes32) {
        return sha3(uint8(1));
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
// Warning: (77-96): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data. The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.
// Warning: (168-182): "sha3" has been deprecated in favour of "keccak256"
// Warning: (168-182): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data. The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.
// Warning: (254-270): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data. The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.
// Warning: (342-361): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data. The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.

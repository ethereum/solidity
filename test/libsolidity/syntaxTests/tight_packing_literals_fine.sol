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
// Warning: (168-182): "sha3" has been deprecated in favour of "keccak256"

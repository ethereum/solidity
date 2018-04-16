contract C {
    function f() pure public returns (bytes32) {
        return keccak256(uint8(1));
    }
    function g() pure public returns (bytes) {
        return abi.encode(1);
    }
    function h() pure public returns (bytes) {
        return abi.encodePacked(uint8(1));
    }
}

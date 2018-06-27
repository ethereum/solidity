contract C {
    function k() pure public returns (bytes) {
        return abi.encodePacked(uint8(1));
    }
    function l() pure public returns (bytes) {
        return abi.encode(1);
    }
}
// ----

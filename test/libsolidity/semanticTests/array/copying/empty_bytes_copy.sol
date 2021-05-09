contract C {
    bytes data;
    bytes otherData;
    function fromMemory() public returns (bytes1) {
        bytes memory t;
        uint[2] memory x;
        x[0] = type(uint).max;
        data = t;
        data.push();
        return data[0];
    }
    function fromCalldata(bytes calldata x) public returns (bytes1) {
        data = x;
        data.push();
        return data[0];
    }
    function fromStorage() public returns (bytes1) {
        // zero-length but dirty higher order bits
        assembly  { sstore(otherData.slot, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00) }
        data = otherData;
        data.push();
        return data[0];
    }
}
// ====
// compileViaYul: also
// ----
// fromMemory() -> 0x00
// fromCalldata(bytes): 0x40, 0x60, 0x00, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff -> 0x00
// fromStorage() -> 0x00

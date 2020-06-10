contract C {
    function to_little_endian_64(uint64 value) public pure returns (bytes memory ret) {
        ret = new bytes(8);
        bytes8 bytesValue = bytes8(value);
        // Byteswapping during copying to bytes.
        ret[0] = bytesValue[7];
        ret[1] = bytesValue[6];
        ret[2] = bytesValue[5];
        ret[3] = bytesValue[4];
        ret[4] = bytesValue[3];
        ret[5] = bytesValue[2];
        ret[6] = bytesValue[1];
        ret[7] = bytesValue[0];
    }
}
// ====
// compileViaYul: also
// ----
// to_little_endian_64(uint64): 0 -> 0x20, 8, 0x00
// to_little_endian_64(uint64): 0x0102030405060708 -> 0x20, 8, 0x0807060504030201000000000000000000000000000000000000000000000000

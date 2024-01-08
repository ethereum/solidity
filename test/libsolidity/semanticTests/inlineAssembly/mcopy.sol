contract C {
    function f(bytes memory src) public pure returns (bytes memory dst) {
        assembly {
            mcopy(add(dst, 0x1f), add(src, 0x1f), 0x01) // Copy over src length byte to dst
            mcopy(add(dst, 0x20), add(src, 0x00), 0x08) // Copy 8 zero bytes to dst
            mcopy(add(dst, 0x28), add(src, 0x28), 0x10) // Copy 16 bytes from the middle of src to dst
            mcopy(add(dst, 0x38), add(src, 0x00), 0x08) // Copy 8 zero bytes to dst
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f(bytes): 0x20, 0x20, 0xffeeddccbbaa9988776655443322110000112233445566778899aabbccddeeff -> 0x20, 0x20, 0x0000000000000000776655443322110000112233445566770000000000000000

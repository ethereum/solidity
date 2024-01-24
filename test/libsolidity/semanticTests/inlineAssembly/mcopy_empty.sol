contract C {
    function mcopy_zero(bytes memory pattern) public pure returns (bytes memory out) {
        out = pattern;

        // This should have no effect on output
        assembly {
            mcopy(add(out, 0x20), add(out, 0x30), 0)
            mcopy(add(out, 0x30), add(out, 0x30), 0)
            mcopy(add(out, 0x40), add(out, 0x30), 0)

            mcopy(add(out, 0x21), 0, 0)
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// mcopy_zero(bytes): 0x20, 0x20, 0xffeeddccbbaa9988776655443322110000112233445566778899aabbccddeeff -> 0x20, 0x20, 0xffeeddccbbaa9988776655443322110000112233445566778899aabbccddeeff

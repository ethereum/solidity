contract C {
    bytes1 b1;
    bytes2 b2;
    uint8 i8 = 8;
    uint16 i16 = 16;

    function f() public {
        // Illegal, different sizes
        bytes1(i16);
        uint8(b2);
        bytes2(i8);
        uint16(b1);
    }
}
// ----
// TypeError: (153-164): Explicit type conversion not allowed from "uint16" to "bytes1".
// TypeError: (174-183): Explicit type conversion not allowed from "bytes2" to "uint8".
// TypeError: (193-203): Explicit type conversion not allowed from "uint8" to "bytes2".
// TypeError: (213-223): Explicit type conversion not allowed from "bytes1" to "uint16".

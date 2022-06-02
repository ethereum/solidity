function e(uint x) pure returns (uint) { return x; }

contract C {
    function f() public pure {
        // Zero and non-zero number
        0 e;
        1 e;
        1000 e;
        115792089237316195423570985008687907853269984665640564039457584007913129639935 e; // 2**256 - 1
        0.0 e;
        0.00 e;
        1.0000 e;
        9999.0 e;

        // Hexadecimal
        0x0 e;
        0x0000 e;
        0x1234 e;
        0xffffff e;

        // Almost address
        0x000012345678901234567890123456789012345678 e;
        0x12345678901234567890123456789012345678 e;

        // Hexadecimal that resembles scientific notation
        0x0e0 e;
        0x111e0 e;
        0xeeeee e;
        0x10e10 e;
        0x10e76 e;

        // Number with separators
        1_2_3_4_5_6_7_8_9_0 e;
        1_000 e;
        1_000_000 e;
        9999_9999_9999 e;
        0x123_abc e;
        1_000.0 e;
        1_000.000_000 e;

        // Scientific notation
        0e0 e;
        0e-0 e;
        0e-1 e;
        0e-10 e;
        1e0 e;
        10e0 e;

        10e10 e;
        10e76 e;

        10e-1 e;
        1000e-0003 e;
        1200e-2 e;

        // Scientific notation with decimals
        0.0e0 e;
        0.0e-0 e;
        0.0e-1 e;
        0.0e-10 e;
        1.0e0 e;
        1.0e2 e;
        10.0e0 e;
        100.0_000e-2 e;
        1.23e2 e;
        0.00115792089237316195423570985008687907853269984665640564039457584007913129639935e80 e; // (2**256 - 1) * 10**-80 * 10**80

        // Scientific notation with separators
        1_0e1_0 e;
        10_000_000_000e1_0 e;
        10_000_000_000e-0_0_0_1_0 e;
    }
}

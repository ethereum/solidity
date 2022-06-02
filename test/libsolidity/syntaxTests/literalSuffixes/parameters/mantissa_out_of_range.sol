function s8(int8 m, uint e) pure returns (uint) {}
function u8(uint8 m, uint e) pure returns (uint) {}
function s16(int16 m, uint e) pure returns (uint) {}
function u16(uint16 m, uint e) pure returns (uint) {}
function s256(int m, uint e) pure returns (uint) {}
function u256(uint m, uint e) pure returns (uint) {}

contract C {
    function max() public pure {
        // TODO: For all of these the error should be that the mantissa or exponent is out of range.
        // Best if we can tell which of them.
        128 s8;
        256 u8;
        32768 s16;
        65536 u16;
        57896044618658097711785492504343953926634992332820282019728792003956564819968 s256; // 2**255
        115792089237316195423570985008687907853269984665640564039457584007913129639936 u256; // 2**256

        1.28 s8;
        2.56 u8;
        3.2768 s16;
        6.5536 u16;
        5.7896044618658097711785492504343953926634992332820282019728792003956564819968 s256; // 2**255 * 10**-76
        1.15792089237316195423570985008687907853269984665640564039457584007913129639936 u256; // 2**256 * 10**-77

        128_000 s8;
        256_000 u8;
        32768_000 s16;
        65536_000 u16;
        57896044618658097711785492504343953926634992332820282019728792003956564819968_000 s256; // 2**255 * 10**-76
        115792089237316195423570985008687907853269984665640564039457584007913129639936_000 u256; // 2**256 * 10**-77
    }
}
// ----
// TypeError 8838: (517-523): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (533-539): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (549-558): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (568-577): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (587-669): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (689-772): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (793-800): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (810-817): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (827-837): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (847-857): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (867-950): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 5503: (980-1064): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function.
// TypeError 8838: (1095-1105): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1115-1125): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1135-1148): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1158-1171): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1181-1267): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1297-1384): The type of the literal cannot be converted to the parameters of the suffix function.

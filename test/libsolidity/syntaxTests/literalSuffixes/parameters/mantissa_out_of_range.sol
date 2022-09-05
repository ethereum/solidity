function mi8eu8(int8 m, uint8 e) pure suffix returns (uint) {}
function s8(int8 m, uint e) pure suffix returns (uint) {}
function u8(uint8 m, uint e) pure suffix returns (uint) {}
function s16(int16 m, uint e) pure suffix returns (uint) {}
function u16(uint16 m, uint e) pure suffix returns (uint) {}
function s256(int m, uint e) pure suffix returns (uint) {}
function u256(uint m, uint e) pure suffix returns (uint) {}

contract C {
    function max() public pure {
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
        57896044618658097711785492504343953926634992332820282019728792003956564819968_000 s256; // 2**255 * 10**3
        115792089237316195423570985008687907853269984665640564039457584007913129639936_000 u256; // 2**256 * 10**3

        128e-256 mi8eu8;
    }
}
// ----
// TypeError 8838: (475-481): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int8.
// TypeError 8838: (491-497): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type uint8.
// TypeError 8838: (507-516): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int16.
// TypeError 8838: (526-535): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type uint16.
// TypeError 8838: (545-627): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int256.
// TypeError 8838: (647-730): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (751-758): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int8.
// TypeError 8838: (768-775): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type uint8.
// TypeError 8838: (785-795): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int16.
// TypeError 8838: (805-815): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type uint16.
// TypeError 8838: (825-908): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int256.
// TypeError 8838: (938-1022): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (1053-1063): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int8.
// TypeError 8838: (1073-1083): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type uint8.
// TypeError 8838: (1093-1106): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int16.
// TypeError 8838: (1116-1129): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type uint16.
// TypeError 8838: (1139-1225): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (1253-1340): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (1369-1384): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int8. The exponent is out of range of type uint8.

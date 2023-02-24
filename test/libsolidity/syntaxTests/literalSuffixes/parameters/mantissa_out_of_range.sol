function s8(int8 m, uint e) pure suffix returns (uint) {}
function u8(uint8 m, uint e) pure suffix returns (uint) {}
function s16(int16 m, uint e) pure suffix returns (uint) {}
function u16(uint16 m, uint e) pure suffix returns (uint) {}
function s256(int m, uint e) pure suffix returns (uint) {}
function u256(uint m, uint e) pure suffix returns (uint) {}

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
// TypeError 8838: (559-565): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (575-581): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (591-600): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (610-619): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (629-711): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 5503: (731-814): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 8838: (835-842): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (852-859): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (869-879): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (889-899): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (909-992): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 5503: (1022-1106): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 8838: (1137-1147): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1157-1167): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1177-1190): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 8838: (1200-1213): The type of the literal cannot be converted to the parameters of the suffix function.
// TypeError 5503: (1223-1309): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 5503: (1339-1426): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.

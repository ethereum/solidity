function mi8eu8(int8 m, uint8 e) pure suffix returns (uint) {}
function u8(uint m, uint8 e) pure suffix returns (uint) {}
function u16(uint m, uint16 e) pure suffix returns (uint) {}
function u256(uint m, uint e) pure suffix returns (uint) {}

contract C {
    function min() public pure {
        // We never use positive exponents so here it's just 0 and the whole number goes into mantissa
        1.0e78 u8;
        1.0e78 u16;
        1.0e78 u256;

        115792089237316195423570985008687907853269984665640564039457584007913129639936 u8;   // 2**256
        115792089237316195423570985008687907853269984665640564039457584007913129639936 u16;  // 2**256
        115792089237316195423570985008687907853269984665640564039457584007913129639936 u256; // 2**256
    }

    function max() public pure {
        1e-256 u8;
        128e-256 mi8eu8;
    }
}
// ----
// TypeError 8838: (401-407): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (420-426): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (440-446): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (462-540): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (565-643): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (668-746): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.
// TypeError 8838: (811-817): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The exponent is out of range of type uint8.
// TypeError 8838: (830-838): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of the suffix function. The mantissa is out of range of type int8. The exponent is out of range of type uint8.

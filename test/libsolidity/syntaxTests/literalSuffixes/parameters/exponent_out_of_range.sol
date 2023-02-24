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
    }
}
// ----
// TypeError 5503: (338-347): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 5503: (357-367): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 5503: (377-388): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 5503: (399-480): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 5503: (502-584): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 5503: (605-688): This fractional number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function.
// TypeError 8838: (748-757): The type of the literal cannot be converted to the parameters of the suffix function.

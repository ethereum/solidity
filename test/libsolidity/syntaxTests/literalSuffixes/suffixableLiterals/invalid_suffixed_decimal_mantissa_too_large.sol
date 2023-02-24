struct Decimal {
    uint mantissa;
    uint exponent;
}

function suffix(uint mantissa, uint exponent) pure suffix returns (Decimal memory) { return Decimal(mantissa, exponent); }

contract C {
    Decimal x = 0.115792089237316195423570985008687907853269984665640564039457584007913129639936 suffix; // 2**256 * 10**-78
}
// ----
// TypeError 8838: (211-291): This number cannot be decomposed into a mantissa and decimal exponent that fit the range of parameters of any possible suffix function. The mantissa is out of range of the largest supported integer type.

pragma abicoder v2;

struct Decimal {
    uint mantissa;
    uint exponent;
}

function asDecimal(uint mantissa, uint exponent) pure suffix returns (Decimal memory) {
    return Decimal(mantissa, exponent);
}

contract C {
    function zero() public pure returns (Decimal memory) {
        return 0.0 asDecimal;
    }

    function simple() public pure returns (Decimal memory, Decimal memory, Decimal memory, Decimal memory, Decimal memory) {
        return (
            1234567800.0 asDecimal,
            12345678.0 asDecimal,
            1234.5678 asDecimal,
            12.345678 asDecimal,
            0.12345678 asDecimal
        );
    }

    function maxMantissa() public pure returns (Decimal memory) {
        return 0.00115792089237316195423570985008687907853269984665640564039457584007913129639935e10 asDecimal;  // (2**256 - 1) * 10**-(80 - 10)
    }

    function maxUint8Exponent() public pure returns (Decimal memory) {
        return 1e-256 asDecimal;
    }
}

// ----
// zero() -> 0, 0
// simple() -> 1234567800, 0, 12345678, 0, 12345678, 4, 12345678, 6, 12345678, 8
// maxMantissa() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 70
// maxUint8Exponent() -> 1, 256

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
        return 0 asDecimal;
    }

    function simple() public pure returns (Decimal memory, Decimal memory, Decimal memory) {
        return (
            1234567800 asDecimal,
            12345678 asDecimal,
            1 asDecimal
        );
    }

    function maxMantissa() public pure returns (Decimal memory) {
        return 115792089237316195423570985008687907853269984665640564039457584007913129639935 asDecimal;  // 2**256 - 1)
    }
}

// ----
// zero() -> 0, 0
// simple() -> 1234567800, 0, 12345678, 0, 1, 0
// maxMantissa() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0

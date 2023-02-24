function u8(uint m, uint8 e) pure suffix returns (uint) {}
function u16(uint m, uint16 e) pure suffix returns (uint) {}
function u256(uint m, uint e) pure suffix returns (uint) {}

contract C {
    function min() public pure {
        // We never use positive exponents so here it's just 0 and the whole number goes into mantissa
        1.0e77 u8;
        1.0e77 u16;
        1.0e77 u256;

        115792089237316195423570985008687907853269984665640564039457584007913129639935.0 u8;   // 2**256 - 1
        115792089237316195423570985008687907853269984665640564039457584007913129639935.0 u16;  // 2**256 - 1
        115792089237316195423570985008687907853269984665640564039457584007913129639935.0 u256; // 2**256 - 1
    }

    function max() public pure {
        1e-255 u8;
    }
}

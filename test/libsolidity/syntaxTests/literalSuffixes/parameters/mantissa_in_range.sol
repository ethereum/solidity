function s8(int8 m, uint e) pure suffix returns (uint) {}
function u8(uint8 m, uint e) pure suffix returns (uint) {}
function s16(int16 m, uint e) pure suffix returns (uint) {}
function u16(uint16 m, uint e) pure suffix returns (uint) {}
function s256(int m, uint e) pure suffix returns (uint) {}
function u256(uint m, uint e) pure suffix returns (uint) {}

contract C {
    function min() public pure {
        0 s8;
        0 u8;
        0 s16;
        0 u16;
        0 s256;
        0 u256;

        0.0 s8;
        0.0 u8;
        0.0 s16;
        0.0 u16;
        0.0 s256;
        0.0 u256;
    }

    function max() public pure {
        127 s8;
        255 u8;
        32767 s16;
        65535 u16;
        57896044618658097711785492504343953926634992332820282019728792003956564819967 s256;  // 2**255 - 1
        115792089237316195423570985008687907853269984665640564039457584007913129639935 u256; // 2**256 - 1

        1.27 s8;
        2.55 u8;
        3.2767 s16;
        6.5535 u16;
        5.7896044618658097711785492504343953926634992332820282019728792003956564819967 s256;  // (2**255 - 1) * 10**-76
        1.15792089237316195423570985008687907853269984665640564039457584007913129639935 u256; // (2**256 - 1) * 10**-77
    }
}

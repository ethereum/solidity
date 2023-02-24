function uSuffix(uint8, uint) pure suffix returns (int8) {}
function uSuffix(uint16, uint) pure suffix returns (int16) {}

contract C {
    int32 a = 115792089237316195423570985008687907853269984665640564039457584007913129639936 uSuffix; // 2**256
}
// ----
// TypeError 9322: (229-236): No matching declaration found after argument-dependent lookup.

function s8(int8 x) pure returns (uint) {}
function u8(uint8 x) pure returns (uint) {}
function s16(int16 x) pure returns (uint) {}
function u16(uint16 x) pure returns (uint) {}
function s256(int x) pure returns (uint) {}
function u256(uint x) pure returns (uint) {}

contract C {
    function max() public pure {
        128 s8;
        256 u8;
        32768 s16;
        65536 u16;
        57896044618658097711785492504343953926634992332820282019728792003956564819968 s256; // 2**255
        115792089237316195423570985008687907853269984665640564039457584007913129639936 u256; // 2**256
    }
}
// ----
// TypeError 8838: (322-328): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (338-344): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (354-363): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (373-382): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (392-474): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (494-577): The type of the literal cannot be converted to the parameter of the suffix function.

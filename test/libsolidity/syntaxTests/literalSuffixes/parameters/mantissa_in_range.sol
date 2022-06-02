function s8(int8 m, uint e) pure returns (uint) {}
function u8(uint8 m, uint e) pure returns (uint) {}
function s16(int16 m, uint e) pure returns (uint) {}
function u16(uint16 m, uint e) pure returns (uint) {}
function s256(int m, uint e) pure returns (uint) {}
function u256(uint m, uint e) pure returns (uint) {}

contract C {
    function min() public pure {
        //0 s8;     // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0 u8;     // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0 s16;    // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0 u16;    // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0 s256;   // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0 u256;   // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.

        //0.0 s8;   // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0.0 u8;   // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0.0 s16;  // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0.0 u16;  // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0.0 s256; // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //0.0 u256; // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
    }

    function max() public pure {
        //127 s8;                                                                                              // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //255 u8;                                                                                              // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //32767 s16;                                                                                           // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //65535 u16;                                                                                           // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //57896044618658097711785492504343953926634992332820282019728792003956564819967 s256; // 2**255 - 1    // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //115792089237316195423570985008687907853269984665640564039457584007913129639935 u256; // 2**256 - 1   // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.

        1.27 s8;
        2.55 u8;
        3.2767 s16;
        6.5535 u16;
        5.7896044618658097711785492504343953926634992332820282019728792003956564819967 s256; // (2**255 - 1) * 10**-76
        1.15792089237316195423570985008687907853269984665640564039457584007913129639935 u256; // (2**256 - 1) * 10**-77
    }
}

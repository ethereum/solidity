contract C {
    function leftU(uint8 x, uint8 y) public returns (uint8) {
        return x << y;
    }

    function leftS(int8 x, int8 y) public returns (int8) {
        return x << y;
    }
}

// ====
// compileViaYul: also
// ----
// leftU(uint8,uint8): 255, 8 -> 0
// leftU(uint8,uint8): 255, 1 -> 254
// leftU(uint8,uint8): 255, 0 -> 255
// leftS(int8,int8): 1, 7 -> -128 # Result is -128 and output is sign-extended, not zero-padded. #
// leftS(int8,int8): 1, 6 -> 64

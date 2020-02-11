contract C {
    function leftU(uint8 x, uint8 y) public returns(uint8) {
        return x << y;
    }

    function leftS(int8 x, int8 y) public returns(int8) {
        return x << y;
    }
}

// ----
// leftU(uint8,uint8): 255, 8 -> 0
// leftU(uint8,uint8):"255, 8" -> "0"
// leftU(uint8,uint8): 255, 1 -> 254
// leftU(uint8,uint8):"255, 1" -> "254"
// leftU(uint8,uint8): 255, 0 -> 255
// leftU(uint8,uint8):"255, 0" -> "255"
// leftS(int8,int8): 1, 7 -> 0 - 128
// leftS(int8,int8):"1, 7" -> "115792089237316195423570985008687907853269984665640564039457584007913129639808"
// leftS(int8,int8): 1, 6 -> 64
// leftS(int8,int8):"1, 6" -> "64"

contract C {
    function l(bytes20 x, uint8 y) public returns(bytes20) {
        return x << y;
    }

    function r(bytes20 x, uint8 y) public returns(bytes20) {
        return x >> y;
    }
}

// ----
// l(bytes20,uint8): "12345678901234567890", 64 -> "901234567890"
// r(bytes20,uint8): "12345678901234567890", 64 -> 0x313233343536373839303132000000000000000000000000

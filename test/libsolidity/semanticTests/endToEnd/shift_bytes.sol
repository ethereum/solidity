contract C {
    function left(bytes20 x, uint8 y) public returns(bytes20) {
        return x << y;
    }

    function right(bytes20 x, uint8 y) public returns(bytes20) {
        return x >> y;
    }
}

// ----
// left(bytes20,uint8): "12345678901234567890", 8 * 8 -> "901234567890" + string(8, 0
// left(bytes20,uint8):"12345678901234567890, 64" -> "901234567890\0\0\0\0\0\0\0\0"
// right(bytes20,uint8): "12345678901234567890", 8 * 8 -> string(8, 0 + "123456789012"
// right(bytes20,uint8):"12345678901234567890, 64" -> "\0\0\0\0\0\0\0\0123456789012"

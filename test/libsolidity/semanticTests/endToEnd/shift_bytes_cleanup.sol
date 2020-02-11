contract C {
    function left(uint8 y) public returns(bytes20) {
        bytes20 x;
        assembly {
            x: = "12345678901234567890abcde"
        }
        return x << y;
    }

    function right(uint8 y) public returns(bytes20) {
        bytes20 x;
        assembly {
            x: = "12345678901234567890abcde"
        }
        return x >> y;
    }
}

// ----
// left(uint8): 8 * 8 -> "901234567890" + string(8, 0
// left(uint8):"64" -> "901234567890\0\0\0\0\0\0\0\0"
// right(uint8): 8 * 8 -> string(8, 0 + "123456789012"
// right(uint8):"64" -> "\0\0\0\0\0\0\0\0123456789012"

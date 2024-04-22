contract C {
    function f(uint y) public pure {
        // one byte too long for storing in Fixedbytes (would require 33 bytes)
        y = 0xffffffff00000000ffffffff00000000ffffffff00000000ffffffff000000001;

        // one bit too long for storing in Fixedbytes (would require 33 bytes)
        y = 0b11111111100000000111111110000000011111111000000001111111100000000111111110000000011111111000000001111111100000000111111110000000011111111000000001111111100000000111111110000000011111111000000001111111100000000111111110000000011111111000000001111111100000000;
    }
}
// ----
// TypeError 7407: (142-209): Type int_const 1852...(71 digits omitted)...7281 is not implicitly convertible to expected type uint256. Literal is too large to fit in uint256.
// TypeError 7407: (303-562): Type int_const 2311...(70 digits omitted)...0416 is not implicitly convertible to expected type uint256. Literal is too large to fit in uint256.

contract C {
    function f(uint y) public pure {
        // one byte too long for storing in Fixedbytes (would require 33 bytes)
        y = 0xffffffff00000000ffffffff00000000ffffffff00000000ffffffff000000001;
    }
}
// ----
// TypeError: (142-209): Type int_const 1852...(71 digits omitted)...7281 is not implicitly convertible to expected type uint256.

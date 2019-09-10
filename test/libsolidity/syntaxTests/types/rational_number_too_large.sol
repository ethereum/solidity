contract c {
    function f() public pure {
        uint8 a = 256;
    }
}
// ----
// TypeError: (52-65): Type int_const 256 is not implicitly convertible to expected type uint8. Literal is too large to fit in uint8.

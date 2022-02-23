contract C {
    function f() pure public {
        uint x;
        (x, ) = (1E111);
    }
}
// ----
// TypeError 7407: (76-83): Type int_const 1000...(104 digits omitted)...0000 is not implicitly convertible to expected type tuple(uint256,).

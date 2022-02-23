contract C {
    function f() internal pure {
        uint i = 31415999999999999999999999999999999999999999999999999999999999999999933**3;
        uint unreachable = 123;
    }
}
// ----
// TypeError 9574: (54-137): Type int_const 3100...(204 digits omitted)...9237 is not implicitly convertible to expected type uint256. Literal is too large to fit in uint256.

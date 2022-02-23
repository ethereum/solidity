contract C {
    function f() public pure {
        uint8 a = 256;
        uint8 b = uint8(256);
        int8 c = int8(-129);
    }
}
// ----
// TypeError 9574: (52-65): Type int_const 256 is not implicitly convertible to expected type uint8. Literal is too large to fit in uint8.
// TypeError 9640: (85-95): Explicit type conversion not allowed from "int_const 256" to "uint8".
// TypeError 9640: (114-124): Explicit type conversion not allowed from "int_const -129" to "int8".

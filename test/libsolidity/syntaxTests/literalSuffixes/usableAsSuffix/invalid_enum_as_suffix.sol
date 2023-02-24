enum E { A, B, C}

contract C {
    uint a = 1000 E;
}
// ----
// TypeError 9640: (45-51): Explicit type conversion not allowed from "int_const 1000" to "enum E".

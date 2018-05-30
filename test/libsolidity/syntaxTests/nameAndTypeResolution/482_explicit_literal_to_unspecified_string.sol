contract C {
    function f() pure public {
        string("abc");
    }
}
// ----
// TypeError: (52-65): Explicit type conversion not allowed from "literal_string "abc"" to "string storage pointer".

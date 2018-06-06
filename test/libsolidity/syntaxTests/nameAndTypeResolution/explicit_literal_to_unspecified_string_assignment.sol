contract C {
    function f() pure public {
        string x = "abc";
    }
}
// ----
// Warning: (52-60): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// TypeError: (52-68): Type literal_string "abc" is not implicitly convertible to expected type string storage pointer.

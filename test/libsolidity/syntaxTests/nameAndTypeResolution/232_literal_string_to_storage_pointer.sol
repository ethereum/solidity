contract C {
    function f() public { string x = "abc"; }
}
// ----
// Warning: (39-47): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// TypeError: (39-55): Type literal_string "abc" is not implicitly convertible to expected type string storage pointer.

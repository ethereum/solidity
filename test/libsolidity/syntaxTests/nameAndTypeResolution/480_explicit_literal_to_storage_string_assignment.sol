contract C {
    function f() pure public {
        string storage x = "abc";
    }
}
// ----
// TypeError 9574: (52-76='string storage x = "abc"'): Type literal_string "abc" is not implicitly convertible to expected type string storage pointer.

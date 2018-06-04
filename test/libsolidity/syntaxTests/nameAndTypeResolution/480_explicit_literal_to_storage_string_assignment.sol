contract C {
    function f() pure public {
        string storage x = "abc";
    }
}
// ----
// TypeError: (52-76): Type literal_string "abc" is not implicitly convertible to expected type string storage pointer.

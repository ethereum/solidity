contract C {
    function f() public { string storage x = "abc"; }
}
// ----
// TypeError: (39-63): Type literal_string "abc" is not implicitly convertible to expected type string storage pointer.

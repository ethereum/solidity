contract C {
    string s = hex"a000";
}
// ----
// TypeError 7407: (28-37): Type literal_string hex"a000" is not implicitly convertible to expected type string storage ref. Contains invalid UTF-8 sequence at position 0.

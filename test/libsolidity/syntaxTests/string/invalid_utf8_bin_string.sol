contract C {
    string s = bin"1010000000000000";
}
// ----
// TypeError 7407: (28-49): Type literal_string hex"a000" is not implicitly convertible to expected type string storage ref. Contains invalid UTF-8 sequence at position 0.

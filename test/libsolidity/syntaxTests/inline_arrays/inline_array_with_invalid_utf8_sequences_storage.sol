contract C {
    string[2] data = [hex'74000001', hex'c0a80101'];
}
// ----
// TypeError 6069: (50-63): Type literal_string hex"c0a80101" is not implicitly convertible to expected type string storage ref. Contains invalid UTF-8 sequence at position 4.

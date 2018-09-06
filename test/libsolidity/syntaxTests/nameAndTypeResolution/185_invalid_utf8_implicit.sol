contract C {
    string s = "\xa0\x00";
}
// ----
// TypeError: (28-38): Type literal_string (contains invalid UTF-8 sequence at position 0) is not implicitly convertible to expected type string storage ref.

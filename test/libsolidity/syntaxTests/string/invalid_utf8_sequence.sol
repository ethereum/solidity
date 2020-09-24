contract C {
    string s = unicode"À";
}
// ----
// SyntaxError 8452: (28-38): Contains invalid UTF-8 sequence at position 0.
// TypeError 7407: (28-38): Type literal_string hex"c0" is not implicitly convertible to expected type string storage ref. Contains invalid UTF-8 sequence at position 0.

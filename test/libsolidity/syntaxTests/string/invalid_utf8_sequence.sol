contract C {
    string s = unicode"À";
}
// ----
// SyntaxError 8452: (28-38): Invalid UTF-8 sequence found
// TypeError 7407: (28-38): Type literal_string (contains invalid UTF-8 sequence at position 0) is not implicitly convertible to expected type string storage ref.

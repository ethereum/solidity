contract C {
    bytes32 b = ~hex"00ff11";
}
// ----
// TypeError 4907: (29-41): Built-in unary operator ~ cannot be applied to type literal_string hex"00ff11".

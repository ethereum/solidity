contract C {
    bytes32 b = ~bin"11111111";
}
// ----
// TypeError 4907: (29-43): Built-in unary operator ~ cannot be applied to type literal_string hex"ff".

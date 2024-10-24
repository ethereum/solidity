object "a.b" {
    code {}
}

// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
// ----
// SyntaxError 9822: In EOF context object name "a.b" must not contain 'dot' character.
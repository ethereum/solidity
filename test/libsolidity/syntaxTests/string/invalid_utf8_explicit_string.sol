contract C {
    string s = string("\xa0\x00");
}
// ----
// TypeError 9640: (28-46): Explicit type conversion not allowed from "literal_string hex"a000"" to "string memory". Contains invalid UTF-8 sequence at position 0.

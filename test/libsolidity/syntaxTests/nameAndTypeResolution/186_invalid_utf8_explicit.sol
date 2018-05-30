contract C {
    string s = string("\xa0\x00");
}
// ----
// TypeError: (28-46): Explicit type conversion not allowed from "literal_string (contains invalid UTF-8 sequence at position 0)" to "string storage pointer".

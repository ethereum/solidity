contract C {
    fallback () internal { }
}
// ----
// TypeError 1159: (17-41): Fallback function must be defined as "external".

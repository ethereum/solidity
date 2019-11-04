contract C {
    fallback () internal { }
}
// ----
// TypeError: (17-41): Fallback function must be defined as "external".

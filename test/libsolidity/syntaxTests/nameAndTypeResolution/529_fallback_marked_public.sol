contract C {
    fallback () public { }
}
// ----
// TypeError 1159: (17-39='fallback () public { }'): Fallback function must be defined as "external".

contract C {
    fallback () internal { }
}
// ----
// TypeError 1159: (17-41='fallback () internal { }'): Fallback function must be defined as "external".

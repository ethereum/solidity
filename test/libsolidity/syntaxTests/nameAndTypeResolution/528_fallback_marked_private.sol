contract C {
    fallback () private { }
}
// ----
// TypeError 1159: (17-40='fallback () private { }'): Fallback function must be defined as "external".

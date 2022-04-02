contract C {
    // Check that visibility is also enforced for the fallback function.
    fallback() {}
}
// ----
// SyntaxError 4937: (90-103='fallback() {}'): No visibility specified. Did you intend to add "external"?
// TypeError 1159: (90-103='fallback() {}'): Fallback function must be defined as "external".

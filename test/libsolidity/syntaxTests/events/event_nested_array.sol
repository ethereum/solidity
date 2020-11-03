contract c {
    event E(uint[][]);
}
// ----
// TypeError 3061: (25-33): This type is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.

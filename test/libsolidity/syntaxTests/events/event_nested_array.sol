contract c {
    event E(uint[][]);
}
// ----
// TypeError: (25-33): This type is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.

function fixedSuffix(fixed) pure suffix returns (uint) {}
function ufixedSuffix(ufixed) pure suffix returns (uint) {}

contract C {
    uint u = 1.1 fixedSuffix;
    uint uf = 1.1 ufixedSuffix;
}
// ----
// TypeError 2699: (21-26): Parameters of fixed-point types are not allowed in literal suffix functions. To support fractional literals the suffix function must accept two integer arguments (mantissa and exponent) that such literals can be decomposed into.
// TypeError 2699: (80-86): Parameters of fixed-point types are not allowed in literal suffix functions. To support fractional literals the suffix function must accept two integer arguments (mantissa and exponent) that such literals can be decomposed into.

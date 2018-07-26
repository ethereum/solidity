library test {
    function f(bytes calldata) internal pure {}
}
// ----
// TypeError: (30-35): Location has to be memory or storage for reference type parameter.

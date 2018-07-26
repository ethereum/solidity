contract test {
    function f(bytes calldata) internal;
}
// ----
// TypeError: (31-36): Location has to be memory or storage for reference type parameter.

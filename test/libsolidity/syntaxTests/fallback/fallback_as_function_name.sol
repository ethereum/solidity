contract C {
    function fallback() external pure {}
}
// ----
// Warning 3445: (26-34): This function is named "fallback" but is not the fallback function of the contract. If you intend this to be a fallback function, use "fallback(...) { ... }" without the "function" keyword to define it.

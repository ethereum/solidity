library test {
    function f(bytes calldata) internal pure {}
}
// ----
// TypeError: (30-35): Variable cannot be declared as "calldata" (remove the "calldata" keyword).

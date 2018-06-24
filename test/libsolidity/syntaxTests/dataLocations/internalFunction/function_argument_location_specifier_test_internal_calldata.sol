contract test {
    function f(bytes calldata) internal;
}
// ----
// TypeError: (31-36): Variable cannot be declared as "calldata" (remove the "calldata" keyword).

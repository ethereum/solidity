{
    function f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) {}
    f(calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0), calldataload(0))
}
// ====
// EVMVersion: >=tangerineWhistle
// stackOptimization: true
// viaSSACFG: true
// ----
// StackTooDeepError: Stack too deep.
// No memoryguard was present. Consider using memory-safe assembly only and annotating it via 'assembly ("memory-safe") { ... }'.

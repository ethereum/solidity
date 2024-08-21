{
    let x := calldataload(0)
    let double := verbatim_1i_1o(hex"600202", x)
    mstore(0, double)
}
// ----
// Trace:
//   Unknown builtin: verbatim_1i_1o
// Memory dump:
// Storage dump:
// Transient storage dump:

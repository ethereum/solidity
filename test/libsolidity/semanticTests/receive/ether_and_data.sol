contract C {
    receive () payable external { }
}
// ====
// compileToEwasm: also
// ----
// (), 1 ether
// (), 1 ether: 1 -> FAILURE

contract C {
    receive () payable external { }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// (), 1 ether
// (), 1 ether: 1 -> FAILURE

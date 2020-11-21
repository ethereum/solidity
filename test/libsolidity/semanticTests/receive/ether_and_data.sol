contract C {
    receive () payable external { }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// (), 1 ether
// (), 1 ether: 1 -> FAILURE

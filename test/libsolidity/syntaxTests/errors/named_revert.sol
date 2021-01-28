error E();
function f() pure {
    revert({error: E()});
}
// ----
// TypeError 1886: (35-55): Named arguments cannot be used for this function call.

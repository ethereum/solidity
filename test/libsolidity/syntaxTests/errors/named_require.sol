error E();
function f() pure {
    require({error: E()});
}
// ----
// TypeError 1886: (35-56): Named arguments cannot be used for this function call.

error E();
function f() pure {
    // TODO is it a problem that we do not get an error here?
    // we should at least get "statement has no effect"
    E;
}
// ----

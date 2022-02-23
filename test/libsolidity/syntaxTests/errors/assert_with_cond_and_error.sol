error E();
function f(bool x) pure {
    assert(x, E());
}
// ----
// TypeError 6160: (41-55): Wrong argument count for function call: 2 arguments given but expected 1.

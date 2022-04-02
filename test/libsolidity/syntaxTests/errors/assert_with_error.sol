error E();
function f() pure {
    assert(E());
}
// ----
// TypeError 9553: (42-45='E()'): Invalid type for argument in function call. Invalid implicit conversion from tuple() to bool requested.

error E();
function f() pure {
    revert((E()));
}
function g() pure {
    bool x;
    require(x, (E()));
}
// ----
// TypeError 6473: (43-46): Tuple component cannot be empty.
// TypeError 4423: (42-47): Expected error instance or string.
// TypeError 6473: (100-103): Tuple component cannot be empty.
// TypeError 4423: (99-104): Expected error instance or string.

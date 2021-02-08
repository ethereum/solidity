error E();

contract C {
    bytes4 t = E.selector;
}
// ----
// TypeError 9582: (40-50): Member "selector" not found or not visible after argument-dependent lookup in function () pure.

contract A {
    function() external public f;
}

contract C {
    bytes4 constant s4 = A.f.selector;
}
// ----
// TypeError 9582: (88-91): Member "f" not found or not visible after argument-dependent lookup in type(contract A).

contract B {
    function() external public g;
}

contract C is B {
    bytes4 constant s4 = super.g.selector;
}
// ----
// TypeError 9582: (93-100): Member "g" not found or not visible after argument-dependent lookup in type(contract super C).

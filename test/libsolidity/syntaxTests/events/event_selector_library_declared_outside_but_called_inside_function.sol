library Y {
    event E() anonymous;
}

contract C {
    bytes32 s5 = Y.E.selector;

    function test2() view external returns (bytes32) {
        return s5;
    }
}
// ----
// TypeError 9582: (70-82): Member "selector" not found or not visible after argument-dependent lookup in function ().

library Y {
    event E() anonymous;
}

contract D {
    function test1() external pure returns (bytes32) {
        return Y.E.selector;
    }
}
// ----
// TypeError 9582: (123-135): Member "selector" not found or not visible after argument-dependent lookup in function ().

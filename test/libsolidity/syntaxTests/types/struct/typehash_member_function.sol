contract C {
    struct S {
        function(uint256) internal returns(uint256) f;
    }

    bytes32 h = type(S).typehash;
}
// ----
// TypeError 9518: (106-122): "typehash" cannot be used for structs with members of "function (uint256) returns (uint256)" type.

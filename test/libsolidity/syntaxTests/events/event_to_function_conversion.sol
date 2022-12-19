
contract C {
    event E(uint);
    function() internal pure x = E;
}
// ----
// TypeError 7407: (66-67): Type event E(uint256) is not implicitly convertible to expected type function () pure. Special functions cannot be converted to function types.

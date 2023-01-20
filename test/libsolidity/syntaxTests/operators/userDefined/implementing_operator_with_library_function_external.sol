type Int is int128;

library L {
    function externalOperator(Int, Int) external pure returns (Int) {}
}

// FIXME: Not being able to use external library functions in 'using for' is a bug.
// https://github.com/ethereum/solidity/issues/13765
using {L.externalOperator as +} for Int;
// ----
// DeclarationError 7920: (251-269): Identifier not found or not unique.

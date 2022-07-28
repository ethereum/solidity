type Int is int128;

library L {
    function externalOperator(Int, Int) external returns (Int) {}
}

// FIXME: Not being able to use external library functions in 'using for' is a bug.
// https://github.com/ethereum/solidity/issues/13765
using {L.externalOperator as +} for Int;
// ----
// DeclarationError 7920: (246-264): Identifier not found or not unique.

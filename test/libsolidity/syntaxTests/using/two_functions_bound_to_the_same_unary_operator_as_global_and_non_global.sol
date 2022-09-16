type Int is int;

using {unsub as -} for Int global;
using {another_unsub as -} for Int;

function unsub(Int) pure returns (Int) {}

function another_unsub(Int) pure returns (Int) {}

function test() pure returns (Int) {
    return -Int.wrap(2);
}

// ----
// TypeError 4907: (232-244): Built-in unary operator - cannot be applied to type Int. Multiple user-defined functions provided for this operator.

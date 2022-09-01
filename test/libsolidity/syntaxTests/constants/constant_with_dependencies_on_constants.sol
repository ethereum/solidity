uint256 constant MAX = 1;

library L1 {
    uint256 internal constant INT = 100;
}

contract C1 {
    uint256 internal constant CONST = 20 + L2.INT; // forward reference
    uint256 internal constant LIMIT = MAX * L1.INT;  // same file & external library constant
    uint256 internal constant NESTED = LIMIT + CONST; // nested & same contract constant
}

contract C2 is C1 {
    uint256 internal constant INHERITED = NESTED + CONST * LIMIT; // inherited constants
}

contract C3 is C2 {
    uint256 internal constant NESTED_INHERITED = INHERITED + NESTED + CONST * LIMIT; // nest-inherited constants
}

library L2 {
    uint256 internal constant INT = 100;
}

// ----

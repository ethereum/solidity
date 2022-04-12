uint256 constant MAX = 1;

library L1 {
    uint256 internal constant INT = 100;
}

contract C1 {
    uint256 internal constant CONST = 20 + L2.INT; // forward reference
    uint256 internal constant LIMIT = MAX * L1.INT;  // same file & external library constant
    uint256 internal constant NESTED = LIMIT + CONST; // nested & same contract constant

    uint256[L1.INT] internal arr1; // error, backward reference
    uint256[L2.INT] internal arr2; // error, forward reference
}

contract C2 is C1 {
    uint256 internal constant INHERITED = NESTED + CONST * LIMIT; // inherited constants
}

contract C3 is C2 {
    uint256 internal constant NESTED_INHERITED = INHERITED + NESTED + CONST * LIMIT; // nest-inherited constants

    uint256[CONST] internal arr3;            // error, nest-inherited constants
    uint256[NESTED_INHERITED] internal arr4; // error, same contract constant
}

library L2 {
    uint256 internal constant INT = 100;
}

// ----
// TypeError 5462: (366-372): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (430-436): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (742-747): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (822-838): Invalid array length, expected integer literal or constant expression.

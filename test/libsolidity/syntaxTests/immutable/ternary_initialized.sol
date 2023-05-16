contract A
{
    uint256 immutable x;
    uint256 y = 0;

    constructor(bool _branch)
    {
        _branch ? x = 1 : y = 2;
        _branch ? y = 3 : x = 4;
    }
}

// ----
// TypeError 4599: (112-113): Cannot write to immutable here: Immutable variables cannot be initialized inside a conditional (if/ternary) statement.
// TypeError 4599: (153-154): Cannot write to immutable here: Immutable variables cannot be initialized inside a conditional (if/ternary) statement.

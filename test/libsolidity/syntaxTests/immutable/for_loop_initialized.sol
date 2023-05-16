contract C {
    uint immutable x;
    constructor() {
        for (x = 1; x < 10; ++x)
            x = 5;
    }
}

// ----
// TypeError 6672: (68-69): Cannot write to immutable here: Immutable variables cannot be initialized inside a loop.
// TypeError 6672: (85-86): Cannot write to immutable here: Immutable variables cannot be initialized inside a loop.
// TypeError 6672: (100-101): Cannot write to immutable here: Immutable variables cannot be initialized inside a loop.

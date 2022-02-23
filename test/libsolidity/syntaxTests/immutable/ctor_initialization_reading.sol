contract C {
    uint immutable x;
    constructor() {
        x = 3 + x;
    }
}
// ----
// TypeError 7733: (71-72): Immutable variables cannot be read before they are initialized.

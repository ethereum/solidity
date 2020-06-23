contract C {
    uint immutable x;
    constructor() {
        x = 1;
        x = 4;
    }
}
// ----
// TypeError 1574: (78-79): Immutable state variable already initialized.

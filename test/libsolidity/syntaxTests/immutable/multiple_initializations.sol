contract C {
    uint immutable x;
    constructor() public {
        x = 1;
        x = 4;
    }
}
// ----
// TypeError: (85-86): Immutable state variable already initialized.

contract C {
    uint immutable x;
    constructor() {
        return;

        x = 1;
    }
}
// ----
// Warning 5740: (80-85): Unreachable code.

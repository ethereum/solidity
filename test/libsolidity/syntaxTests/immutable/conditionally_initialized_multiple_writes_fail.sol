contract C {
    uint immutable multipleWrites;

    constructor() {
        if (true) {
            multipleWrites = 1;
        } else {
            multipleWrites = 1;
        }
        multipleWrites = 2; // error
    }
}
// ----
// TypeError 1574: (188-202): Immutable state variable already initialized.

contract C {
    uint immutable multipleWrites;

    constructor() {
        if (true) {
            multipleWrites = 1;
        } else {
            multipleWrites = 2;
        }
    }
}
// ----

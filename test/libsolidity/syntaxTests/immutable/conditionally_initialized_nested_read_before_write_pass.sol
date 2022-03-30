contract C {
    uint immutable nestedReadBeforeWrite;

    constructor() {
        if (true) {
            nestedReadBeforeWrite = 1;
        } else {
            if (true) {
                nestedReadBeforeWrite = 1;
            } else {
                nestedReadBeforeWrite = 1;
                nestedReadBeforeWrite + 42; // good
            }
        }
    }
}
// ----

contract C {
    uint immutable readBeforeWrite;

    constructor() {
        if (true) {
            readBeforeWrite = 1;
        } else {
            readBeforeWrite = 1;
            readBeforeWrite + 42; // good
        }
    }
}
// ----

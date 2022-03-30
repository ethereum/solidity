contract C {
    uint immutable readBeforeWrite;

    constructor() {
        if (true) {
            readBeforeWrite = 1;
        } else {
            readBeforeWrite + 42; // error
            readBeforeWrite = 1;
        }
    }
}
// ----
// TypeError 7733: (152-167): Immutable variables cannot be read before they are initialized.

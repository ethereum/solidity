contract C {
    uint immutable nestedReadBeforeWrite;

    constructor() {
        if (true) {
            nestedReadBeforeWrite = 1;
        } else {
            if (true) {
                nestedReadBeforeWrite = 1;
            } else {
                nestedReadBeforeWrite + 42; // error
                nestedReadBeforeWrite = 1;
            }
        }
    }
}
// ----
// TypeError 7733: (256-277): Immutable variables cannot be read before they are initialized.

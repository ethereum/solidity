contract C {
    uint immutable nonAssignInitInBranch;

    constructor() {
        if (true) {
            nonAssignInitInBranch = 1;
        } else {
            nonAssignInitInBranch = 2;
            delete nonAssignInitInBranch; // modify error
        }
    }
}
// ----
// TypeError 2718: (210-231): Immutable variables cannot be modified after initialization.

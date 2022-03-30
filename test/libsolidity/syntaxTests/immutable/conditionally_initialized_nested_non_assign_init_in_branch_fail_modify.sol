contract C {
    uint immutable nestedNonAssignInitInBranch;

    constructor() {
        if (true) {
            nestedNonAssignInitInBranch = 1;
        } else {
            if (true) {
                nestedNonAssignInitInBranch = 1;
            } else {
                nestedNonAssignInitInBranch = 1;
                delete nestedNonAssignInitInBranch; // modify error
            }
        }
    }
}
// ----
// TypeError 2718: (330-357): Immutable variables cannot be modified after initialization.

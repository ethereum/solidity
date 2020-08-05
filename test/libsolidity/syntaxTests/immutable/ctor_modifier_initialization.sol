contract C {
    uint immutable x;
    constructor() initX {
    }

    modifier initX() {
        _; x = 23;
    }
}
// ----
// TypeError 1581: (102-103): Immutable variables can only be initialized inline or assigned directly in the constructor.

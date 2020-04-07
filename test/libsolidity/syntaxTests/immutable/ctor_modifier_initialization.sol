contract C {
    uint immutable x;
    constructor() initX public {
    }

    modifier initX() {
        _; x = 23;
    }
}
// ----
// TypeError: (109-110): Immutable variables can only be initialized inline or assigned directly in the constructor.

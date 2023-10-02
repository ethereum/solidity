contract C {
    uint immutable x;
    constructor() {
        x = 3 + x;
    }
}

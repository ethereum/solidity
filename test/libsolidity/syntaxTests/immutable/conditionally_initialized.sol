contract C {
    uint immutable x;
    constructor() {
        if (false)
            x = 1;
    }
}

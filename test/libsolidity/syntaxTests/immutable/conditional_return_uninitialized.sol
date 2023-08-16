contract C {
    uint immutable x;
    constructor() {
        if (false)
            return;

        x = 1;
    }
}

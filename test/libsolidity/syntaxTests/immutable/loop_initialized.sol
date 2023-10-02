contract C {
    uint immutable x;
    constructor() {
        while (true)
            x = 1;
    }
}

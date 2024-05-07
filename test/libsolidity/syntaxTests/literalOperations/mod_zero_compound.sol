contract A {
    uint a;
    constructor() { a = 5; a %= 0; }
}
// ----

contract A {
    uint a = 5;
    constructor() { a %= uint(((2)*2)%4); }
}
// ----

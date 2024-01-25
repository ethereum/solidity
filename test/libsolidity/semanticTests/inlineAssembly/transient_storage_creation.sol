contract C {
    constructor() {
        uint x;
        assembly {
            tstore(0, 42)
            x := tload(0)
        }
        assert(x == 42);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// constructor() ->

contract C {
    constructor() {
        assembly ("memory-safe") {}
    }

    function f() public {
        assembly ("memory-safe") {}
    }
}
// ----
// :C(creation) true
// :C(runtime) true

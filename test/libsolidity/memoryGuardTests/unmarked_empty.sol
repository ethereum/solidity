contract C {
    constructor() {
        assembly {}
    }

    function f() public {
        assembly {}
    }
}
// ----
// :C(creation) true
// :C(runtime) true

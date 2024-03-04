contract C {
    constructor() {
        assembly ("memory-safe") {
            mstore(0, 1)
        }
    }

    function store() public {
        assembly ("memory-safe") {
            mstore(0, 1)
        }
    }
}
// ----
// :C(creation) true
// :C(runtime) true

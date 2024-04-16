contract C {
    constructor() {
        assembly {
            mstore(0, 1)
        }
    }

    function store() public {
        assembly {
            mstore(0, 1)
        }
    }
}
// ----
// :C(creation) false
// :C(runtime) false

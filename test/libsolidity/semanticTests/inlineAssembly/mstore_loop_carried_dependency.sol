contract C {
    uint public result;

    // mstore() below must survive UnusedStoreEliminator; reproduces with the default
    // optimizer sequence (only detected when run with --optimize).
    function run() external {
        assembly {
            let ptr := mload(0x40)
            let r := 0
            for { let i := 0 } lt(i, 3) { i := add(i, 1) } {
                // reads the word written by the previous iteration
                let b := add(ptr, 0)
                r := add(r, mload(b))
                // writes the word read by the next iteration
                let a := add(ptr, 0x20)
                let v := add(i, 1)
                mstore(a, v)
                ptr := add(ptr, 0x20)
            }
            sstore(0, r)
            return(0, 0)
        }
    }
}
// ----
// run() ->
// result() -> 3

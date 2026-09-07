object "UnusedStoreEliminatorNoSSA" {
    code {
        let x := calldataload(4)
        let a := add(x, 32)
        x := add(x, 32)
        let b := x
        let outLen := 32
        // This mstore must survive: a and b are equal (both x's old value + 32),
        // so it IS read by return; reassigning x just hides that from USE.
        mstore(a, 0xAA)
        return(b, outLen)
    }
}

object "a" {
    code {
        let addr1 := linkersymbol("contract/test.sol:L1")
        let addr2 := linkersymbol("contract/test.sol:L2")
        sstore(0, addr1)
        sstore(1, addr2)
    }
}

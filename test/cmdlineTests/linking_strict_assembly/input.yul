object "a" {
    code {
        let addr := linkersymbol("contract/test.sol:L")
        sstore(0, addr)
    }
}

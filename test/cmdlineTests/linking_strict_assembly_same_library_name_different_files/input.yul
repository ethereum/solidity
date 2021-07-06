object "a" {
    code {
        let addr1 := linkersymbol("library1.sol:L")
        let addr2 := linkersymbol("library2.sol:L")
        sstore(0, addr1)
        sstore(1, addr2)
    }
}

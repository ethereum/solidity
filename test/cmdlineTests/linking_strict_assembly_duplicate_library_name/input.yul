object "a" {
    code {
        let addr := linkersymbol("library.sol:L")
        sstore(0, addr)
    }
}

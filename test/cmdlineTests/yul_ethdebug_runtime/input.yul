object "object" {
    code {
        let a
        let b
        {
            function z() -> y
            { y := calldataload(0) }
            a := z()
        }
        {
            function z() -> y
            { y := calldataload(0x20) }
            b := z()
        }
        sstore(a, b)
    }
}
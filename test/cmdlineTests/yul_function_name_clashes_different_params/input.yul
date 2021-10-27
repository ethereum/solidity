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
            function z(r) -> y
            { y := calldataload(r) }
            b := z(0x70)
        }
        sstore(a, b)
    }
}

object "object" {
    code {
        {
            /// @debug.set {"scope": 1}
            let a
            /// @debug.set {"scope": 1, "assignment": 1}
            a := z()
            /// @debug.set {"scope": 1}
            let b
            /// @debug.set {"scope": 1, "assignment": 2}
            b := z_1()
            /// @debug.set {"scope": 1}
            sstore(a, b)
            /// @debug.set {}
        }
        function z() -> y
        {
            /// @debug.set {"scope": 2}
            y := calldataload(0)
            /// @debug.set {}
        }
        function z_1() -> y
        {
            /// @debug.set {"scope": 3}
            y := calldataload(0x20)
            /// @debug.set {}
        }
    }
}

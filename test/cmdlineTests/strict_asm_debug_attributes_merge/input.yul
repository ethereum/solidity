object "object" {
    code {
        {
            /// @debug.merge {"scope": 1}
            let a
            /// @debug.merge {"assignment": 1}
            a := z()
            /// @debug.merge {"assignment": null}
            let b
            /// @debug.merge {"assignment": 2}
            b := z_1()
            /// @debug.merge {"assignment": null}
            sstore(a, b)
            /// @debug.merge {"scope": null}
        }
        function z() -> y
        {
            /// @debug.merge {"scope": 2}
            y := calldataload(0)
            /// @debug.merge {"scope": null}
        }
        function z_1() -> y
        {
            /// @debug.merge {"scope": 3}
            y := calldataload(0x20)
            /// @debug.merge {"scope": null}
        }
    }
}

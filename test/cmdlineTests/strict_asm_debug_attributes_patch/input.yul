object "object" {
    code {
        {
            /// @debug.patch  {"op": "add", "path": "/scope", "value": 1}
            let a
            /// @debug.patch  {"op": "add", "path": "/assignment", "value": 1}
            a := z()
            /// @debug.patch  {"op": "remove", "path": "/assignment"}
            let b
            /// @debug.patch  {"op": "add", "path": "/assignment", "value": 2}
            b := z_1()
            /// @debug.patch  {"op": "remove", "path": "/assignment"}
            sstore(a, b)
            /// @debug.patch  {"op": "remove", "path": "/scope"}
        }
        function z() -> y
        {
            /// @debug.patch  {"op": "add", "path": "/scope", "value": 2}
            y := calldataload(0)
            /// @debug.patch  {"op": "remove", "path": "/scope"}
        }
        function z_1() -> y
        {
            /// @debug.patch  {"op": "add", "path": "/scope", "value": 3}
            y := calldataload(0x20)
            /// @debug.patch  {"op": "remove", "path": "/scope"}
        }
    }
}

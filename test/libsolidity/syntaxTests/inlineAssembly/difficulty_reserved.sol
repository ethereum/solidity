contract C {
    function f() public view returns (uint ret) {
        assembly {
            let difficulty := sload(0)
            ret := difficulty
        }
    }
    function g() public pure returns (uint ret) {
        assembly {
            function difficulty() -> r {
                r := 1000
            }
            ret := difficulty()
        }
    }
}
// ----
// ParserError 5568: (98-108): Cannot use builtin function name "difficulty" as identifier name.

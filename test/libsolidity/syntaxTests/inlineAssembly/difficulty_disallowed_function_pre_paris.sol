contract C {
    function f() public view returns (uint256 ret) {
        assembly {
            let difficulty := sload(0)
            ret := difficulty
        }
    }

    function g() public pure returns (uint256 ret) {
        assembly {
            function difficulty() -> r {
                r := 1000
            }
            ret := difficulty()
        }
    }
}
// ====
// EVMVersion: <paris
// ----
// ParserError 5568: (101-111): Cannot use builtin function name "difficulty" as identifier name.

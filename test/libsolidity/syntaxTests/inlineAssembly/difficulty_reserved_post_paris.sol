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
// EVMVersion: >=paris
// ----
// DeclarationError 5017: (101-111): The identifier "difficulty" is reserved and can not be used.
// DeclarationError 5017: (255-323): The identifier "difficulty" is reserved and can not be used.

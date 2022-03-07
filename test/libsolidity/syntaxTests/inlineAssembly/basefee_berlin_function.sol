contract C {
    function f() public view returns (uint ret) {
        assembly {
            let basefee := sload(0)
            ret := basefee
        }
    }
    function g() public pure returns (uint ret) {
        assembly {
            function basefee() -> r {
                r := 1000
            }
            ret := basefee()
        }
    }
}
// ====
// EVMVersion: =berlin
// ----
// DeclarationError 5017: (98-105): The identifier "basefee" is reserved and can not be used.
// DeclarationError 5017: (242-307): The identifier "basefee" is reserved and can not be used.

contract C {
    function f() public pure {
        assembly {
            function swapn(dupn) -> exchange {}
            let dupn
        }
    }
    function g() public pure {
        assembly {
            function dupn(exchange) -> swapn {}
            let exchange
        }
    }
    function h() public pure {
        assembly {
            function exchange(swapn) -> dupn {}
            let swapn
        }
    }
}
// ====
// EVMVersion: >=amsterdam
// ----
// DeclarationError 5017: (75-110): The identifier "swapn" is reserved and can not be used.
// DeclarationError 5017: (90-94): The identifier "dupn" is reserved and can not be used.
// Warning 5470: (99-107): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// DeclarationError 5017: (127-131): The identifier "dupn" is reserved and can not be used.
// DeclarationError 5017: (210-245): The identifier "dupn" is reserved and can not be used.
// Warning 5470: (224-232): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// DeclarationError 5017: (237-242): The identifier "swapn" is reserved and can not be used.
// Warning 5470: (262-270): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (349-384): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// DeclarationError 5017: (367-372): The identifier "swapn" is reserved and can not be used.
// DeclarationError 5017: (377-381): The identifier "dupn" is reserved and can not be used.
// DeclarationError 5017: (401-406): The identifier "swapn" is reserved and can not be used.

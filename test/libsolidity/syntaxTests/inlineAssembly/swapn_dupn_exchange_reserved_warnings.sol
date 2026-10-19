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
// ----
// Warning 5470: (75-110): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (90-94): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (99-107): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (127-131): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (210-245): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (224-232): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (237-242): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (262-270): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (349-384): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (367-372): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (377-381): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (401-406): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.

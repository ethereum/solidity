object "test"
{
    code {}
    object "test0"
    {
        code
        {
            function swapn(dupn) -> exchange {}
        }
    }

    object "test1"
    {
        code
        {
            function dupn(exchange) -> swapn {}
        }
    }

    object "test2"
    {
        code
        {
            function exchange(swapn) -> dupn {}
        }
    }

    object "test3"
    {
        code
        {
            let swapn
            let dupn
            let exchange
        }
    }
}
// ====
// EVMVersion: >=amsterdam
// ----
// DeclarationError 5017: (88-123): The identifier "swapn" is reserved and can not be used.
// DeclarationError 5017: (103-107): The identifier "dupn" is reserved and can not be used.
// Warning 5470: (112-120): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// DeclarationError 5017: (201-236): The identifier "dupn" is reserved and can not be used.
// Warning 5470: (215-223): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// DeclarationError 5017: (228-233): The identifier "swapn" is reserved and can not be used.
// Warning 5470: (314-349): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// DeclarationError 5017: (332-337): The identifier "swapn" is reserved and can not be used.
// DeclarationError 5017: (342-346): The identifier "dupn" is reserved and can not be used.
// DeclarationError 5017: (431-436): The identifier "swapn" is reserved and can not be used.
// DeclarationError 5017: (453-457): The identifier "dupn" is reserved and can not be used.
// Warning 5470: (474-482): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.

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
// EVMVersion: <amsterdam
// ----
// Warning 5470: (88-123): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (103-107): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (112-120): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (201-236): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (215-223): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (228-233): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (314-349): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (332-337): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (342-346): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (431-436): "swapn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (453-457): "dupn" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.
// Warning 5470: (474-482): "exchange" will be promoted to Yul reserved identifier in the future and will not be allowed anymore as an identifier.

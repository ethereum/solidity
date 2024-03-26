error CustomError(string);

contract C
{
    function f() external pure
    {
        require(false, CustomError("errorReason"));
    }
}

// ====
// compileViaYul: true
// ----
// f() -> FAILURE, hex"8d6ea8be", hex"0000000000000000000000000000000000000000000000000000000000000020", hex"000000000000000000000000000000000000000000000000000000000000000b", hex"6572726f72526561736f6e000000000000000000000000000000000000000000"

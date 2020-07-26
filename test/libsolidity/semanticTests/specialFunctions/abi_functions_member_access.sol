contract C {
    function f() public pure {
        abi.encode;
        abi.encodePacked;
        abi.encodeWithSelector;
        abi.encodeWithSignature;
        abi.decode;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() ->

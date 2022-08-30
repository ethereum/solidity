library Lib { function m() public returns (address) { return msg.sender; } }
contract Test {
    address public sender;
    function f() public {
        sender = Lib.m();
    }
}
// ====
// compileToEwasm: false
// EVMVersion: >=homestead
// ----
// library: Lib
// f() ->
// sender() -> 0x1212121212121212121212121212120000000012

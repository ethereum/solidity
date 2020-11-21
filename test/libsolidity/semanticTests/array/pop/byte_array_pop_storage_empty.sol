contract c {
    bytes data;
    function test() public {
        data.push(0x07);
        data.push(0x05);
        data.push(0x03);
        data.pop();
        data.pop();
        data.pop();
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test() ->
// storage: empty

contract c {
    function set() public returns (bool) { data = msg.data; return true; }
    function checkIfDataIsEmpty() public returns (bool) { return data.length == 0; }
    function sendMessage() public returns (bool, bytes memory) { bytes memory emptyData; return address(this).call(emptyData);}
    fallback() external { data = msg.data; }
    bytes data;
}
// ====
// EVMVersion: >=byzantium
// compileToEwasm: false
// compileViaYul: also
// ----
// (): 1, 2, 3, 4, 5 ->
// gas irOptimized: 155170
// gas legacy: 155251
// gas legacyOptimized: 155214
// checkIfDataIsEmpty() -> false
// sendMessage() -> true, 0x40, 0
// checkIfDataIsEmpty() -> true

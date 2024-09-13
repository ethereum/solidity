contract C {
    address transient a;
    function f() public returns (uint) {
        a = msg.sender;
        return a.balance;
    }
    function g() public returns (uint) {
        return a.balance;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// constructor() ->
// gas legacy: 59027
// gas legacy code: 70400
// account: 0 -> 0x1212121212121212121212121212120000000012
// balance: 0x1212121212121212121212121212120000000012 -> 1267650600228229401496703205376
// f() -> 1267650600228229401496703205376
// g() -> 0
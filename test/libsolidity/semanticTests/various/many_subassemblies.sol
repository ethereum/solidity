contract C0 {}
contract C1 {}
contract C2 {}
contract C3 {}
contract C4 {}
contract C5 {}
contract C6 {}
contract C7 {}
contract C8 {}
contract C9 {}
contract C10 {}

contract D {
    function run() public {
        // This is primarily meant to test assembly import via --import-asm-json.
        // The exported JSON will fail the reimport unless the subassembly indices are parsed
        // correctly - as hex numbers.
        new C0{salt: hex"00"}();
        new C1{salt: hex"01"}();
        new C2{salt: hex"02"}();
        new C3{salt: hex"03"}();
        new C4{salt: hex"04"}();
        new C5{salt: hex"05"}();
        new C6{salt: hex"06"}();
        new C7{salt: hex"07"}();
        new C8{salt: hex"08"}();
        new C9{salt: hex"09"}();
        new C10{salt: hex"0a"}();
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// run() ->
// gas irOptimized: 375189
// gas irOptimized code: 6600
// gas legacy: 375404
// gas legacy code: 17600
// gas legacyOptimized: 375464
// gas legacyOptimized code: 17600
// gas ssaCFGOptimized: 375182
// gas ssaCFGOptimized code: 6600

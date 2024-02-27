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
        new C0();
        new C1();
        new C2();
        new C3();
        new C4();
        new C5();
        new C6();
        new C7();
        new C8();
        new C9();
        new C10();
    }
}
// ----
// run() ->
// gas irOptimized: 375015
// gas irOptimized code: 6600
// gas legacy: 375119
// gas legacy code: 17600
// gas legacyOptimized: 375119
// gas legacyOptimized code: 17600

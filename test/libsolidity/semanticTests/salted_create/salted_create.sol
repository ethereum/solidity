contract B
{
}

contract A {
    function different_salt() public returns (bool) {
        B x = new B{salt: "abc"}();
        B y = new B{salt: "abcef"}();
        return x != y;
    }
    function same_salt() public returns (bool) {
        B x = new B{salt: "xyz"}();
        try new B{salt: "xyz"}() {} catch {
            return true;
        }
        return false;
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// different_salt() -> true
// same_salt() -> true
// gas irOptimized: 98438295
// gas irOptimized code: 600
// gas legacy: 98437509
// gas legacy code: 1600
// gas legacyOptimized: 98437367
// gas legacyOptimized code: 1600

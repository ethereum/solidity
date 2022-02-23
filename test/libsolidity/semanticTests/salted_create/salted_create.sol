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
// compileViaYul: also
// ----
// different_salt() -> true
// same_salt() -> true
// gas irOptimized: 98438914
// gas legacy: 98439116
// gas legacyOptimized: 98438970

contract B
{
    uint x;
    function getBalance() public view returns (uint) {
        return address(this).balance * 1000 + x;
    }
    constructor(uint _x) payable {
        x = _x;
    }
}

contract A {
    function f() public payable returns (uint, uint, uint) {
        B x = new B{salt: "abc", value: 3}(7);
        B y = new B{value: 3, salt: "abc"}(8);
        B z = new B{salt: "abc", value: 3}(9);
        return (x.getBalance(), y.getBalance(), z.getBalance());
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// f(), 10 ether -> 3007, 3008, 3009
// gas irOptimized: 187022
// gas irOptimized code: 67200
// gas legacy: 190858
// gas legacy code: 190200
// gas legacyOptimized: 187256
// gas legacyOptimized code: 92400

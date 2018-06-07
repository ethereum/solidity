contract test {
    uint256 public data;
    bytes6 public name;
    bytes32 public a_hash;
    address public an_address;
    function test() {
        data = 8;
        name = "Celina";
        a_hash = keccak256("\x7b");
        an_address = address(0x1337);
        super_secret_data = 42;
    }
    uint256 super_secret_data;
}
// ----
// data()
// -> 8
// name()
// -> "Celina"
// a_hash()
// -> keccak256(unpadded(123))
// an_address()
// -> 0x1337
// super_secret_data()
// REVERT

contract D {
    uint public x;
    constructor(uint a) {
        x = a;
    }
}

contract C {
    function createDSalted(bytes32 salt, uint arg) public {
        address predictedAddress = address(uint160(uint(keccak256(abi.encodePacked(
            bytes1(0xff),
            address(this),
            salt,
            keccak256(abi.encodePacked(
                type(D).creationCode,
                arg
            ))
        )))));

        D d = new D{salt: salt}(arg);
        require(address(d) == predictedAddress, "Address mismatch.");
    }
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: also
// ----
// createDSalted(bytes32,uint256): 42, 64 ->
// gas legacy: 104365

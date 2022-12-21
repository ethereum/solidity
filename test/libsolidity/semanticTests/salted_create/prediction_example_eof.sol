contract D {
    uint public x;
    constructor(uint a) {
        x = a;
    }
}

// TODO: this is horrible and hopefully avoided at the spec level
function adjustContractCodeForArgSize(bytes memory x, uint16 argSize)
{
  assembly {
	let memPos := add(x, 32)
        if eq(shr(232, mload(memPos)), 0xef0001) {
		let numCodeSections := shr(240, mload(add(memPos, 7)))
		let dataSectionSizeOffset := add(memPos, add(10, mul(numCodeSections, 2)))
		let tmp := mload(dataSectionSizeOffset)
		let dataSectionSize := shr(240, tmp)
		dataSectionSize := add(dataSectionSize, argSize)
		if gt(dataSectionSize, 0xFFFF) { revert(0,0) }
		mstore(dataSectionSizeOffset, or(shr(16, shl(16, tmp)), shl(240, dataSectionSize)))
	}
  }
}

contract C {
    function createDSalted(bytes32 salt, uint arg) public {
        bytes memory creationCode = type(D).creationCode;
	adjustContractCodeForArgSize(creationCode, 32);
        address predictedAddress = address(uint160(uint(keccak256(abi.encodePacked(
            bytes1(0xff),
            address(this),
            salt,
            keccak256(abi.encodePacked(
                creationCode,
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
// gas irOptimized: 100021
// gas legacy: 104455

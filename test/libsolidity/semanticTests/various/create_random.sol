contract C {
    function addr() external returns (address) {
        return address(this);
    }

    function testRunner() external returns (address a1, address a2) {
        assembly {
            // This is `return(0, 1)`. We are using a simplified/fixed initcode to avoid
            // instability due to metadata changes.
            let initcode := hex"60016000f3"
            mstore(0, initcode)

            a1 := create(0, 0, 5)
            a2 := create2(0, 0, 5, address())
        }
    }

    function testCalc() external returns (address a1, address a2) {
        a1 = calculateCreate(address(this), 1);
        a2 = calculateCreate2(address(this), keccak256(hex"60016000f3"), bytes32(uint256(uint160(address(this)))));
    }

    function calculateCreate(address from, uint256 nonce) private pure returns (address) {
        assert(nonce <= 127);
        bytes memory data =
            bytes.concat(hex"d694", bytes20(uint160(from)), nonce == 0 ? bytes1(hex"80") : bytes1(uint8(nonce)));
        return address(uint160(uint256(keccak256(data)))); // Take the lower 160-bits
    }

    function calculateCreate2(address creator, bytes32 codehash, bytes32 salt) private pure returns (address) {
        return address(uint160(uint256(keccak256(abi.encodePacked(bytes1(0xff), creator, salt, codehash)))));
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// addr() -> 0xc06afe3a8444fc0004668591e8306bfb9968e79e
// testRunner() -> 0x137aa4dfc0911524504fcd4d98501f179bc13b4a, 0x2c1c30623ddd93e0b765a6caaca0c859eeb0644d
// testCalc() -> 0x137aa4dfc0911524504fcd4d98501f179bc13b4a, 0x2c1c30623ddd93e0b765a6caaca0c859eeb0644d

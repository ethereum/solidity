contract test {
    function memory1() public returns (uint256 value) {
        assembly {
            mstore(0, 0xda22ac3939b096875fa653a5a89d1231c36f9a4bd50ac53523d04e769b76d5a8)
            value := mload(0)
        }
    }
    function memory2() public returns (uint256 value) {
        assembly {
            mstore(0, 0xda22ac3939b096875fa653a5a89d1231c36f9a4bd50ac53523d04e769b76d5a8)
            value := mload(1)
        }
    }
    function memory3() public returns (uint256 value) {
        assembly {
            mstore(0, 0xda22ac3939b096875fa653a5a89d1231c36f9a4bd50ac53523d04e769b76d5a8)
            mstore(1, 0x233f49fb11fa11436df57dba0f8d50c438395116bc6c36536fbb01bc2ad7043a)
            value := mload(0)
        }
    }
    function memory4() public returns (uint256 value) {
        address a = 0x3f5CE5FBFe3E9af3971dD833D26bA9b5C936f0bE;
        assembly {
            mstore(0, a)
            value := mload(0)
        }
    }
    function storage1() public returns (uint256 value) {
        assembly {
            sstore(0x233f49fb11fa11436df57dba0f8d50c438395116bc6c36536fbb01bc2ad7043a, 0xda22ac3939b096875fa653a5a89d1231c36f9a4bd50ac53523d04e769b76d5a8)
            value := sload(0x233f49fb11fa11436df57dba0f8d50c438395116bc6c36536fbb01bc2ad7043a)
        }
    }
    function storage2() public returns (uint256 value) {
        address a = 0x3f5CE5FBFe3E9af3971dD833D26bA9b5C936f0bE;
        assembly {
            sstore(0x233f49fb11fa11436df57dba0f8d50c438395116bc6c36536fbb01bc2ad7043a, a)
            value := sload(0x233f49fb11fa11436df57dba0f8d50c438395116bc6c36536fbb01bc2ad7043a)
        }
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// memory1() -> 0xda22ac3939b096875fa653a5a89d1231c36f9a4bd50ac53523d04e769b76d5a8
// memory2() -> 0x22ac3939b096875fa653a5a89d1231c36f9a4bd50ac53523d04e769b76d5a800
// memory3() -> 0xda233f49fb11fa11436df57dba0f8d50c438395116bc6c36536fbb01bc2ad704
// memory4() -> 0x3f5CE5FBFe3E9af3971dD833D26bA9b5C936f0bE
// storage1() -> 0xda22ac3939b096875fa653a5a89d1231c36f9a4bd50ac53523d04e769b76d5a8
// storage2() -> 0x3f5CE5FBFe3E9af3971dD833D26bA9b5C936f0bE


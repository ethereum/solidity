contract C {
    uint public x;
    function set(uint _x) public {
        x = _x;
    }
    function get() public view returns (uint) {
        return x;
    }
}
// ----
// .compilation.compiler.name: solc
// .compilation.id: <IGNORE>
// .compilation.sources | length: 1
// .compilation.sources[0].id: 0
// .compilation.sources[0].path: <IGNORE>
// .compilation.sources[0].contents: <IGNORE>
// .compilation.sources[0].language: Solidity
// .resources.compilation.id: <IGNORE>
// .resources.compilation.sources | length: 1
// .resources.compilation.sources[0].id: 0
// .resources.compilation.sources[0].language: Solidity
// .resources.types.t_uint256.kind: uint
// .resources.types.t_uint256.bits: 256
// .resources.pointers | length: 1
//
// C.contract.name: C
// C.creation.environment: create
// C.contract.definition.source.id: 0
// C.creation.instructions[0].offset: 0
// C.creation.instructions[0].operation.mnemonic: PUSH1
// C.creation.instructions[0].operation.arguments[0]: 0x80
// C.creation.instructions[2].operation.mnemonic: MSTORE
//
// C.runtime.contract.name: C
// C.runtime.environment: call

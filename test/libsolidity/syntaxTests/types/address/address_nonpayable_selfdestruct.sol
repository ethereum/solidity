contract C {
    function f(address a) public {
        selfdestruct(a);
    }
}
// ----
// Warning 5159: (56-68): "selfdestruct" has been deprecated. Note that, starting from the Cancun hard fork, the underlying opcode no longer deletes the code and data associated with an account and only transfers its Ether to the beneficiary, unless executed in the same transaction in which the contract was created (see EIP-6780). Any use in newly deployed contracts is strongly discouraged even if the new behavior is taken into account. Future changes to the EVM might further reduce the functionality of the opcode.
// TypeError 9553: (69-70): Invalid type for argument in function call. Invalid implicit conversion from address to address payable requested.

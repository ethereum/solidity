// SPDX-License-Identifier: Apache-2.0

// Implements the following issues:
// - https://github.com/ethereum/solidity/issues/3044
// - https://github.com/ethereum/solidity/issues/4910
// - and `.balance` which is part of the language
//
// TODO: change to "free functions" and import them explicitly

library Contract {
  function balance(address account) internal view returns (uint ret) {
    assembly {
      ret := balance(account)
    }
  }

  function getCode(address account) internal view returns (bytes memory ret) {
    // TODO: optimise for non-ext code case?
    uint size = codesize(account);
    ret = new bytes(size);
    assembly {
      extcodecopy(account, add(ret, 32), 0, size)
    }
  }

  function codesize(address account) internal view returns (uint ret) {
    assembly {
      ret := extcodesize(account)
    }
  }

  function codehash(address account) internal view returns (bytes32 ret) {
    assembly {
      ret := extcodehash(account)
    }
  }

  function exists(address account) internal view returns (bool ret) {
    // According to EIP-1052 + EIP-161.
    assembly {
      ret := iszero(extcodehash(account))
    }
  }

  function hasCode(address account) internal view returns (bool ret) {
    // According to EIP-1052 + EIP-161.
    assembly {
      ret := eq(extcodehash(account), 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470)
    }
  }

  function transfer_(address payable account, uint256 amount) internal {
    // Set stipend.
    // TODO: expose evmVersion here
    uint256 gas_limit = (amount == 0) ? 2300 : 0;
    assembly {
      let success := call(gas_limit, account, amount, 0, 0, 0, 0)
      // TODO: implement revert forwarding
      if iszero(success) { revert(0, 0) }
    }
  }
  
  function send(address payable account, uint256 amount) internal returns (bool success) {
    // Set stipend.
    // TODO: expose evmVersion here
    uint256 gas_limit = (amount == 0) ? 2300 : 0;
    assembly {
      success := call(gas_limit, account, amount, 0, 0, 0, 0)
    }
  }
  
  // TODO: make this a template
  function call(address account, bytes memory input) internal returns (bool success, bytes memory output) {
    uint256 output_len;
    assembly {
      success := call(gas(), account, 0, add(input, 32), mload(input), 0, 0)
      output_len := returndatasize()
    }
    if (success) {
      output = new bytes(output_len);
      assembly {
        returndatacopy(add(output, 32), 0, output_len)
      }
    }
  }

  function delegatecall(address account, bytes memory input) internal returns (bool success, bytes memory output) {
    uint256 output_len;
    assembly {
      success := delegatecall(gas(), account,  add(input, 32), mload(input), 0, 0)
      output_len := returndatasize()
    }
    if (success) {
      output = new bytes(output_len);
      assembly {
        returndatacopy(add(output, 32), 0, output_len)
      }
    }
  }

  function staticcall(address account, bytes memory input) internal returns (bool success, bytes memory output) {
    uint256 output_len;
    assembly {
      success := delegatecall(gas(), account,  add(input, 32), mload(input), 0, 0)
      output_len := returndatasize()
    }
    if (success) {
      output = new bytes(output_len);
      assembly {
        returndatacopy(add(output, 32), 0, output_len)
      }
    }
  }
}

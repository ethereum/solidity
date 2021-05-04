// SPDX-License-Identifier: Apache-2.0

function blockhash(uint blockNumber) returns (bytes32 ret) {
  assembly {
    ret := blockhash(blockNumber)
  }
}

function gasleft() returns (uint256 ret) {
  assembly {
    ret := gas()
  }
}

library Block {
  function coinbase() internal view returns (address payable ret) {
    assembly {
      ret := coinbase()
    }
  }

  function difficulty() internal view returns (uint ret) {
    assembly {
      ret := difficulty()
    }
  }

  function gaslimit() internal view returns (uint ret) {
    assembly {
      ret := gaslimit()
    }
  }

  function number() internal view returns (uint ret) {
    assembly {
      ret := number()
    }
  }

  function timestamp() internal view returns (uint ret) {
    assembly {
      ret := timestamp()
    }
  }
}

library Msg {
  // This can't be calldata here.
  function data() internal view returns (bytes memory ret) {
    uint len;
    assembly {
      len := calldatasize()
    }

    ret = new bytes(len);
    assembly {
      calldatacopy(add(ret, 32), 0, len)
    }
  }
}

/*
msg.data (bytes calldata): complete calldata
msg.sender (address payable): sender of the message (current call)
msg.sig (bytes4): first four bytes of the calldata (i.e. function identifier)
msg.value (uint): number of wei sent with the message
tx.gasprice (uint): gas price of the transaction
tx.origin (address payable): sender of the transaction (full call chain)
*/

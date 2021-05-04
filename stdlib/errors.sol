// SPDX-License-Identifier: Apache-2.0

function revert() {
  assembly {
    revert(0, 0)
  }
}

function revert(string memory reason) {
  assembly {
    revert(add(reason, 32), mload(reason))
  }
}

function assert(bool condition) {
  if (!condition)
    assembly {
      invalid()
    }
}

function require(bool condition) {
  if (!condition)
    revert();
}

function require(bool condition, string memory reason) {
  if (!condition)
    revert(reason);
}

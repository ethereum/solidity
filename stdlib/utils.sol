// SPDX-License-Identifier: Apache-2.0

function min<T>(T a, T b) pure internal returns (T ret) {
  ret = (a < b) ? a : b;
}

function max<T>(T a, T b) pure internal returns (T ret) {
  ret = (a > b) ? a : b;
}

function abs<T: T.isSigned>(T a) pure internal returns (T ret) {
  ret = (a < 0) ? -a : a;
}

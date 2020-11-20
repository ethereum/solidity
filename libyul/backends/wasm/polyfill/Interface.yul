/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

// NOTE: This file is used to generate `ewasmPolyfills/Interface.h`.

function address() -> z1, z2, z3, z4 {
	eth.getAddress(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function balance(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	mstore_address(0:i32, x1, x2, x3, x4)
	eth.getExternalBalance(12:i32, 48:i32)
	z1, z2, z3, z4 := mload_internal(32:i32)
}

function selfbalance() -> z1, z2, z3, z4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}

function chainid() -> z1, z2, z3, z4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}

function origin() -> z1, z2, z3, z4 {
	eth.getTxOrigin(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function caller() -> z1, z2, z3, z4 {
	eth.getCaller(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function callvalue() -> z1, z2, z3, z4 {
	eth.getCallValue(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function calldataload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	calldatacopy(0, 0, 0, 0, x1, x2, x3, x4, 0, 0, 0, 32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function calldatasize() -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(eth.getCallDataSize())
}

function calldatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	let cds:i32 := eth.getCallDataSize()
	let destination:i32 := u256_to_i32(x1, x2, x3, x4)
	let offset:i32 := u256_to_i32(y1, y2, y3, y4)
	let requested_size:i32 := u256_to_i32(z1, z2, z3, z4)
	// overflow?
	if i32.gt_u(offset, i32.sub(0xffffffff:i32, requested_size)) {
		eth.revert(0:i32, 0:i32)
	}

	let available_size:i32 := i32.sub(cds, offset)
	if i32.gt_u(offset, cds) {
		available_size := 0:i32
	}

	if i32.gt_u(available_size, 0:i32) {
		eth.callDataCopy(
			destination,
			offset,
			available_size
		)
	}

	if i32.gt_u(requested_size, available_size) {
		memset(i32.add(destination, available_size), 0:i32, i32.sub(requested_size, available_size))
	}
}

// Needed?
function codesize() -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(eth.getCodeSize())
}

function codecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.codeCopy(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}

function datacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	// TODO correct?
	codecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4)
}

function gasprice() -> z1, z2, z3, z4 {
	eth.getTxGasPrice(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function extcodesize_internal(x1, x2, x3, x4) -> r:i32 {
	mstore_address(0:i32, x1, x2, x3, x4)
	r := eth.getExternalCodeSize(12:i32)
}

function extcodesize(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(extcodesize_internal(x1, x2, x3, x4))
}

function extcodehash(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}

function extcodecopy(a1, a2, a3, a4, p1, p2, p3, p4, o1, o2, o3, o4, l1, l2, l3, l4) {
	mstore_address(0:i32, a1, a2, a3, a4)
	let codeOffset:i32 := u256_to_i32(o1, o2, o3, o4)
	let codeLength:i32 := u256_to_i32(l1, l2, l3, l4)
	eth.externalCodeCopy(12:i32, to_internal_i32ptr(p1, p2, p3, p4), codeOffset, codeLength)
}

function returndatasize() -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(eth.getReturnDataSize())
}

function returndatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.returnDataCopy(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}

function blockhash(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	let r:i32 := eth.getBlockHash(u256_to_i64(x1, x2, x3, x4), 0:i32)
	if i32.eqz(r) {
		z1, z2, z3, z4 := mload_internal(0:i32)
	}
}

function coinbase() -> z1, z2, z3, z4 {
	eth.getBlockCoinbase(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function timestamp() -> z1, z2, z3, z4 {
	z4 := eth.getBlockTimestamp()
}

function number() -> z1, z2, z3, z4 {
	z4 := eth.getBlockNumber()
}

function difficulty() -> z1, z2, z3, z4 {
	eth.getBlockDifficulty(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}

function gaslimit() -> z1, z2, z3, z4 {
	z4 := eth.getBlockGasLimit()
}

function mload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	z1, z2, z3, z4 := mload_internal(to_internal_i32ptr(x1, x2, x3, x4))
}

function mload_internal(pos:i32) -> z1, z2, z3, z4 {
	z1 := bswap64(i64.load(pos))
	z2 := bswap64(i64.load(i32.add(pos, 8:i32)))
	z3 := bswap64(i64.load(i32.add(pos, 16:i32)))
	z4 := bswap64(i64.load(i32.add(pos, 24:i32)))
}

function mstore(x1, x2, x3, x4, y1, y2, y3, y4) {
	mstore_internal(to_internal_i32ptr(x1, x2, x3, x4), y1, y2, y3, y4)
}

function mstore_internal(pos:i32, y1, y2, y3, y4) {
	i64.store(pos, bswap64(y1))
	i64.store(i32.add(pos, 8:i32), bswap64(y2))
	i64.store(i32.add(pos, 16:i32), bswap64(y3))
	i64.store(i32.add(pos, 24:i32), bswap64(y4))
}

function mstore_address(pos:i32, a1, a2, a3, a4) {
	a1, a2, a3 := u256_to_address(a1, a2, a3, a4)
	mstore_internal(pos, 0, a1, a2, a3)
}

function mstore8(x1, x2, x3, x4, y1, y2, y3, y4) {
	let v := u256_to_byte(y1, y2, y3, y4)
	i64.store8(to_internal_i32ptr(x1, x2, x3, x4), v)
}

// Needed?
function msize() -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}

function sload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	mstore_internal(0:i32, x1, x2, x3, x4)
	eth.storageLoad(0:i32, 32:i32)
	z1, z2, z3, z4 := mload_internal(32:i32)
}

function sstore(x1, x2, x3, x4, y1, y2, y3, y4) {
	mstore_internal(0:i32, x1, x2, x3, x4)
	mstore_internal(32:i32, y1, y2, y3, y4)
	eth.storageStore(0:i32, 32:i32)
}

function gas() -> z1, z2, z3, z4 {
	z4 := eth.getGasLeft()
}

function log0(p1, p2, p3, p4, s1, s2, s3, s4) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		0:i32, 0:i32, 0:i32, 0:i32, 0:i32
	)
}

function log1(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		1:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		0:i32, 0:i32, 0:i32
	)
}

function log2(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4,
	t2_1, t2_2, t2_3, t2_4
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		2:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		to_internal_i32ptr(t2_1, t2_2, t2_3, t2_4),
		0:i32, 0:i32
	)
}

function log3(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4,
	t2_1, t2_2, t2_3, t2_4,
	t3_1, t3_2, t3_3, t3_4
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		3:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		to_internal_i32ptr(t2_1, t2_2, t2_3, t2_4),
		to_internal_i32ptr(t3_1, t3_2, t3_3, t3_4),
		0:i32
	)
}

function log4(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4,
	t2_1, t2_2, t2_3, t2_4,
	t3_1, t3_2, t3_3, t3_4,
	t4_1, t4_2, t4_3, t4_4,
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		4:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		to_internal_i32ptr(t2_1, t2_2, t2_3, t2_4),
		to_internal_i32ptr(t3_1, t3_2, t3_3, t3_4),
		to_internal_i32ptr(t4_1, t4_2, t4_3, t4_4)
	)
}

function create(
	x1, x2, x3, x4,
	y1, y2, y3, y4,
	z1, z2, z3, z4
) -> a1, a2, a3, a4 {
	let v1, v2 := u256_to_u128(x1, x2, x3, x4)
	mstore_internal(0:i32, 0, 0, v1, v2)

	let r:i32 := eth.create(0:i32, to_internal_i32ptr(y1, y2, y3, y4), u256_to_i32(z1, z2, z3, z4), 32:i32)
	if i32.eqz(r) {
		a1, a2, a3, a4 := mload_internal(32:i32)
	}
}

function call(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4,
	g1, g2, g3, g4
) -> x1, x2, x3, x4 {
	let g := u256_to_i64(a1, a2, a3, a4)
	mstore_address(0:i32, b1, b2, b3, b4)

	let v1, v2 := u256_to_u128(c1, c2, c3, c4)
	mstore_internal(32:i32, 0, 0, v1, v2)

	x4 := i64.extend_i32_u(eth.call(g, 12:i32, 32:i32, to_internal_i32ptr(d1, d2, d3, d4), u256_to_i32(e1, e2, e3, e4)))
}

function callcode(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4,
	g1, g2, g3, g4
) -> x1, x2, x3, x4 {
	mstore_address(0:i32, b1, b2, b3, b4)

	let v1, v2 := u256_to_u128(c1, c2, c3, c4)
	mstore_internal(32:i32, 0, 0, v1, v2)

	x4 := i64.extend_i32_u(eth.callCode(
		u256_to_i64(a1, a2, a3, a4),
		12:i32,
		32:i32,
		to_internal_i32ptr(d1, d2, d3, d4),
		u256_to_i32(e1, e2, e3, e4)
	))
}

function delegatecall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {
	mstore_address(0:i32, b1, b2, b3, b4)

	x4 := i64.extend_i32_u(eth.callDelegate(
		u256_to_i64(a1, a2, a3, a4),
		12:i32,
		to_internal_i32ptr(c1, c2, c3, c4),
		u256_to_i32(d1, d2, d3, d4)
	))
}

function staticcall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {
	mstore_address(0:i32, b1, b2, b3, b4)

	x4 := i64.extend_i32_u(eth.callStatic(
		u256_to_i64(a1, a2, a3, a4),
		12:i32,
		to_internal_i32ptr(c1, c2, c3, c4),
		u256_to_i32(d1, d2, d3, d4)
	))
}

function create2(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4
) -> x1, x2, x3, x4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}

function selfdestruct(a1, a2, a3, a4) {
	mstore_address(0:i32, a1, a2, a3, a4)
	// In EVM, addresses are padded to 32 bytes, so discard the first 12.
	eth.selfDestruct(12:i32)
}

function return(x1, x2, x3, x4, y1, y2, y3, y4) {
	eth.finish(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4)
	)
}

function revert(x1, x2, x3, x4, y1, y2, y3, y4) {
	eth.revert(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4)
	)
}

function invalid() {
	unreachable()
}

function stop() {
	eth.finish(0:i32, 0:i32)
}

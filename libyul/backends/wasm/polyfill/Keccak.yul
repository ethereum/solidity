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

// NOTE: This file is used to generate `ewasmPolyfills/Keccak.h`.

// reset the context
function keccak_reset(context_offset:i32) {
	// state context
	//	typedef struct {
	//		union {                                 // state:
	//			uint8_t b[200];                     // 8-bit bytes
	//			uint64_t q[25];                     // 64-bit words
	//		} st;
	//		int pt, rsiz, mdlen;                    // these don't overflow
	//	} sha3_ctx_t;
	memset(i32.add(context_offset, 0:i32), 0:i32, 200:i32)
	//	c->mdlen = mdlen;
	//	c->rsiz = 200 - 2 * mdlen;
	//	c->pt = 0;
	i64.store(mdlen_offset(context_offset), 32)
	i64.store(rsiz_offset(context_offset), 136) // 200 - 2 * mdlen = 136
	i64.store(pt_offset(context_offset), 0)
}
function mdlen_offset(context_offset:i32) -> offset:i32 {
	offset :=  i32.add(context_offset, 200:i32)
}
function rsiz_offset(context_offset:i32) -> offset:i32 {
	offset :=  i32.add(context_offset, 208:i32)
}
function pt_offset(context_offset:i32) -> offset:i32 {
	offset :=  i32.add(context_offset, 216:i32)
}
function rounds_consts_offset(context_offset:i32) -> offset:i32 {
	offset :=  i32.add(context_offset, 224:i32)
}
function keccakf_rotc_offset(context_offset:i32) -> offset:i32 {
	offset :=  i32.add(context_offset, 416:i32)
}
function keccakf_piln_offset(context_offset:i32) -> offset:i32 {
	offset :=  i32.add(context_offset, 440:i32)
}
function bc_offset(context_offset:i32) -> offset:i32 {
	offset :=  i32.add(context_offset, 464:i32)
}
// initialize the context
// The context is laid out as follows:
//   0: 1600 bits - 200 bytes - hashing state
// 200:   64 bits - 8 bytes   - mdlen
// 208:   64 bits - 8 bytes   - rsiz
// 216:   64 bits - 8 bytes   - pt
// 224: 1536 bits - 192 bytes - round constants
// 416:  192 bits -  24 bytes - keccakf_rotc
// 440:  192 bits -  24 bytes - keccakf_piln
// 464:  320 bits -  40 bytes - uint64_t bc[5]
function keccak_init(context_offset:i32) {
	keccak_reset(context_offset)
	let round_consts:i32 := rounds_consts_offset(context_offset)
	i64.store(i32.add(round_consts, 0:i32), 0x0000000000000001)
	i64.store(i32.add(round_consts, 8:i32), 0x0000000000008082)
	i64.store(i32.add(round_consts, 16:i32), 0x800000000000808A)
	i64.store(i32.add(round_consts, 24:i32), 0x8000000080008000)
	i64.store(i32.add(round_consts, 32:i32), 0x000000000000808B)
	i64.store(i32.add(round_consts, 40:i32), 0x0000000080000001)
	i64.store(i32.add(round_consts, 48:i32), 0x8000000080008081)
	i64.store(i32.add(round_consts, 56:i32), 0x8000000000008009)
	i64.store(i32.add(round_consts, 64:i32), 0x000000000000008A)
	i64.store(i32.add(round_consts, 72:i32), 0x0000000000000088)
	i64.store(i32.add(round_consts, 80:i32), 0x0000000080008009)
	i64.store(i32.add(round_consts, 88:i32), 0x000000008000000A)
	i64.store(i32.add(round_consts, 96:i32), 0x000000008000808B)
	i64.store(i32.add(round_consts, 104:i32), 0x800000000000008B)
	i64.store(i32.add(round_consts, 112:i32), 0x8000000000008089)
	i64.store(i32.add(round_consts, 120:i32), 0x8000000000008003)
	i64.store(i32.add(round_consts, 128:i32), 0x8000000000008002)
	i64.store(i32.add(round_consts, 136:i32), 0x8000000000000080)
	i64.store(i32.add(round_consts, 144:i32), 0x000000000000800A)
	i64.store(i32.add(round_consts, 152:i32), 0x800000008000000A)
	i64.store(i32.add(round_consts, 160:i32), 0x8000000080008081)
	i64.store(i32.add(round_consts, 168:i32), 0x8000000000008080)
	i64.store(i32.add(round_consts, 176:i32), 0x0000000080000001)
	i64.store(i32.add(round_consts, 184:i32), 0x8000000080008008)
	//    const int keccakf_rotc[24] = {
	//        1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
	//        27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
	//    };
	let keccakf_rotc:i32 := keccakf_rotc_offset(context_offset)
	i64.store(i32.add(keccakf_rotc, 0:i32), bswap64(0x0103060A0F151C24))
	i64.store(i32.add(keccakf_rotc, 8:i32), bswap64(0x2D37020E1B293808))
	i64.store(i32.add(keccakf_rotc, 16:i32), bswap64(0x192B3E12273D142C))
	//    const int keccakf_piln[24] = {
	//        10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
	//        15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
	//    };
	let keccakf_piln:i32 := keccakf_piln_offset(context_offset)
	i64.store(i32.add(keccakf_piln, 0:i32), bswap64(0x0A070B1112030510))
	i64.store(i32.add(keccakf_piln, 8:i32), bswap64(0x081518040F17130D))
	i64.store(i32.add(keccakf_piln, 16:i32), bswap64(0x0C02140E16090601))
	let bc:i32 := bc_offset(context_offset)
	memset(i32.add(bc, 0:i32), 0:i32, 40:i32)
}
function keccak_update(context_offset:i32, input_offset:i32, input_length:i32) {
	//	size_t i;
	//	int j;
	//	j = c->pt;
	//	for (i = 0; i < len; i++) {
	//		c->st.b[j++] ^= ((const uint8_t *) data)[i];
	//		if (j >= c->rsiz) {
	//			keccakf(c->st.q);
	//			j = 0;
	//		}
	//	}
	//	c->pt = j;
	let j:i32 := i32.wrap_i64(i64.load(pt_offset(context_offset)))
	for { let i:i32 := 0:i32 } i32.lt_u(i, input_length) { i := i32.add(i, 1:i32) }
	{
		let ptr:i32 := i32.add(context_offset, j)
		j := i32.add(j, 1:i32)
		i8_store(ptr, i64.xor(i8_load(ptr), i8_load(i32.add(input_offset, i))))
		if i32.ge_u(j, i32.wrap_i64(i64.load(rsiz_offset(context_offset))))
		{
			keccakf(context_offset)
			j := 0:i32
		}
	}
	i64.store(pt_offset(context_offset), i64.extend_i32_u(j))
}
function keccak_finish(context_offset:i32) {
	// c->st.b[c->pt] |= 0x01; // for keccak <- we want to use this.
	let ptr:i32 := i32.add(context_offset, i32.wrap_i64(i64.load(pt_offset(context_offset))))
	i8_store(ptr, i64.or(i8_load(ptr), 0x01))
	// c->st.b[c->rsiz - 1] ^= 0x80
	ptr := i32.add(context_offset, i32.sub(i32.wrap_i64(i64.load(rsiz_offset(context_offset))), 1:i32))
	i8_store(ptr, i64.xor(i8_load(ptr), 0x80))
	// keccakf(c->st.q);
	keccakf(context_offset)
}
function keccak256(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	let input_offset:i32 := u256_to_i32(x1, x2, x3, x4)
	let input_length:i32 := u256_to_i32(y1, y2, y3, y4)
//	debug.print32(input_length)
//	debug.print32(input_offset)
//	debug.print64(i64.load(input_offset))
//	debug.print64(i64.load(i32.add(input_offset, 8:i32)))
//	debug.print64(i64.load(i32.add(input_offset, 16:i32)))
//	debug.print64(i64.load(i32.add(input_offset, 24:i32)))
//	debug.print64(i64.load(i32.add(input_offset, 24:i32)))
//	debug.print64(i64.load(i32.add(input_offset, 32:i32)))
//	debug.print64(i64.load(i32.add(input_offset, 40:i32)))
//	debug.print64(i64.load(i32.add(input_offset, 48:i32)))
//	let input_offset:i32 := 0xE000:i32
//	i64.store(input_offset, 0x6f6f66) // -> "foo"
//	let input_length:i32 := 3:i32
	let context_offset:i32 := 0xF000:i32
	keccak_init(context_offset)
	keccak_update(context_offset, input_offset, input_length)
	keccak_finish(context_offset)
	z1, z2, z3, z4 := mload_internal(context_offset)
}
function rotl64(x, y) -> z {
	// #define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
	z := i64.or(i64.shl(x, y), i64.shr_u(x, i64.sub(64:i64, y)))
}
function keccakf(context_offset:i32) {
	let bc_0:i32 := bc_offset(context_offset)                  // bc[0]:i64
	let bc_1:i32 := i32.add(bc_offset(context_offset), 8:i32)  // bc[1]:i64
	let bc_2:i32 := i32.add(bc_offset(context_offset), 16:i32) // bc[2]:i64
	let bc_3:i32 := i32.add(bc_offset(context_offset), 32:i32) // bc[3]:i64
	let bc_4:i32 := i32.add(bc_offset(context_offset), 64:i32) // bc[4]:i64
	for { let r:i32 := 0:i32 } i32.lt_u(r, 24:i32) { r := i32.add(r, 1:i32) }
	{
		// Theta
		// for (i = 0; i < 5; i++)
		//   bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
		//
		// bc[0] = st[0x0] ^ st[0x5] ^ st[0xa] ^ st[0xf] ^ st[0x14];
		i64.store(bc_0, i64.xor(i64.load(i32.add(0x00:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x28:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x50:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x78:i32, context_offset)),
								i64.load(i32.add(0xa0:i32, context_offset)))))))
		// bc[1] = st[0x1] ^ st[0x6] ^ st[0xb] ^ st[0x10] ^ st[0x15];
		i64.store(bc_1, i64.xor(i64.load(i32.add(0x08:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x30:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x58:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x80:i32, context_offset)),
								i64.load(i32.add(0xa8:i32, context_offset)))))))
		// bc[2] = st[0x2] ^ st[0x7] ^ st[0xc] ^ st[0x11] ^ st[0x16];
		i64.store(bc_2, i64.xor(i64.load(i32.add(0x10:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x38:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x60:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x88:i32, context_offset)),
								i64.load(i32.add(0xb0:i32, context_offset)))))))
		// bc[3] = st[0x3] ^ st[0x8] ^ st[0xd] ^ st[0x12] ^ st[0x17];
		i64.store(bc_3, i64.xor(i64.load(i32.add(0x18:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x40:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x68:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x90:i32, context_offset)),
								i64.load(i32.add(0xb8:i32, context_offset)))))))
		// bc[4] = st[0x4] ^ st[0x9] ^ st[0xe] ^ st[0x13] ^ st[0x18];
		i64.store(bc_4, i64.xor(i64.load(i32.add(0x20:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x48:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x70:i32, context_offset)),
						i64.xor(i64.load(i32.add(0x98:i32, context_offset)),
								i64.load(i32.add(0xC0:i32, context_offset)))))))
		//  for (i = 0; i < 5; i++) {
		//    t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
		//    for (j = 0; j < 25; j += 5)
		//      st[j + i] ^= t;
		//  }
		// t_0 = bc[4] ^ ROTL64(bc[1], 1)
		let t_0 := i64.xor(i64.load(bc_4), rotl64(i64.load(bc_1), 1:i64))
		// t_1 = bc[0] ^ ROTL64(bc[2], 1)
		let t_1 := i64.xor(i64.load(bc_0), rotl64(i64.load(bc_2), 1:i64))
		// t_2 = bc[1] ^ ROTL64(bc[3], 1)
		let t_2 := i64.xor(i64.load(bc_1), rotl64(i64.load(bc_3), 1:i64))
		// t_3 = bc[2] ^ ROTL64(bc[4], 1)
		let t_3 := i64.xor(i64.load(bc_2), rotl64(i64.load(bc_4), 1:i64))
		// t_4 = bc[3] ^ ROTL64(bc[0], 1)
		let t_4 := i64.xor(i64.load(bc_3), rotl64(i64.load(bc_0), 1:i64))
		// st[0] ^= t_0
		// st[1] ^= t_1
		// st[2] ^= t_2
		// st[3] ^= t_3
		mstore_internal(i32.add(context_offset, 0x00:i32),
			bswap64(i64.xor(i64.load(i32.add(0x00:i32, context_offset)), t_0)),
			bswap64(i64.xor(i64.load(i32.add(0x08:i32, context_offset)), t_1)),
			bswap64(i64.xor(i64.load(i32.add(0x10:i32, context_offset)), t_2)),
			bswap64(i64.xor(i64.load(i32.add(0x18:i32, context_offset)), t_3)))
		// st[4] ^= t_4
		// st[5] ^= t_0
		// st[6] ^= t_1
		// st[7] ^= t_2
		mstore_internal(i32.add(context_offset, 0x20:i32),
			bswap64(i64.xor(i64.load(i32.add(0x20:i32, context_offset)), t_4)),
			bswap64(i64.xor(i64.load(i32.add(0x28:i32, context_offset)), t_0)),
			bswap64(i64.xor(i64.load(i32.add(0x30:i32, context_offset)), t_1)),
			bswap64(i64.xor(i64.load(i32.add(0x38:i32, context_offset)), t_2)))
		// st[8] ^= t_3
		// st[9] ^= t_4
		// st[a] ^= t_0
		// st[b] ^= t_1
		mstore_internal(i32.add(context_offset, 0x40:i32),
			bswap64(i64.xor(i64.load(i32.add(0x40:i32, context_offset)), t_3)),
			bswap64(i64.xor(i64.load(i32.add(0x48:i32, context_offset)), t_4)),
			bswap64(i64.xor(i64.load(i32.add(0x50:i32, context_offset)), t_0)),
			bswap64(i64.xor(i64.load(i32.add(0x58:i32, context_offset)), t_1)))
		// st[c] ^= t_2
		// st[d] ^= t_3
		// st[e] ^= t_4
		// st[f] ^= t_0
		mstore_internal(i32.add(context_offset, 0x60:i32),
			bswap64(i64.xor(i64.load(i32.add(0x60:i32, context_offset)), t_2)),
			bswap64(i64.xor(i64.load(i32.add(0x68:i32, context_offset)), t_3)),
			bswap64(i64.xor(i64.load(i32.add(0x70:i32, context_offset)), t_4)),
			bswap64(i64.xor(i64.load(i32.add(0x78:i32, context_offset)), t_0)))
		// st[10] ^= t_1
		// st[11] ^= t_2
		// st[12] ^= t_3
		// st[13] ^= t_4
		mstore_internal(i32.add(context_offset, 0x80:i32),
			bswap64(i64.xor(i64.load(i32.add(0x80:i32, context_offset)), t_1)),
			bswap64(i64.xor(i64.load(i32.add(0x88:i32, context_offset)), t_2)),
			bswap64(i64.xor(i64.load(i32.add(0x90:i32, context_offset)), t_3)),
			bswap64(i64.xor(i64.load(i32.add(0x98:i32, context_offset)), t_4)))
		// st[14] ^= t_0
		// st[15] ^= t_1
		// st[16] ^= t_2
		// st[17] ^= t_3
		mstore_internal(i32.add(context_offset, 0xA0:i32),
			bswap64(i64.xor(i64.load(i32.add(0xA0:i32, context_offset)), t_0)),
			bswap64(i64.xor(i64.load(i32.add(0xA8:i32, context_offset)), t_1)),
			bswap64(i64.xor(i64.load(i32.add(0xB0:i32, context_offset)), t_2)),
			bswap64(i64.xor(i64.load(i32.add(0xB8:i32, context_offset)), t_3)))
		// st[18] ^= t_4
		mstore_internal(i32.add(context_offset, 0xC0:i32),
			bswap64(i64.xor(i64.load(i32.add(0xC0:i32, context_offset)), t_4)), 0x00, 0x00, 0x00)
		// Rho Pi
		// t = st[1];
		// for (i = 0; i < 24; i++)
		// {
		//   j = keccakf_piln[i];
		//   bc[0] = st[j];
		//   st[j] = ROTL64(t, keccakf_rotc[i]);
		//   t = bc[0];
		// }
		let t := i64.load(i32.add(0x08:i32, context_offset))
		for { let i:i32 := 0:i32 } i32.lt_u(i, 24:i32) { i := i32.add(i, 1:i32) }
		{
			let keccakf_piln_i:i32 := i32.add(keccakf_piln_offset(context_offset), i)
			let j:i32 := i8_load32(keccakf_piln_i)
			let st_j:i32 := i32.add(i32.mul(8:i32, j), context_offset)
			i64.store(bc_0, i64.load(st_j))
			i64.store(st_j, rotl64(t, i8_load(i32.add(keccakf_rotc_offset(context_offset), i))))
			t := i64.load(bc_0)
		}
		// Chi
		// for (j = 0; j < 25; j += 5)
		// {
		//   for (i = 0; i < 5; i++)
		//     bc[i] = st[j + i];
		//   for (i = 0; i < 5; i++)
		//     st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
		//}
		for { let j:i32 := 0:i32 } i32.lt_u(j, 25:i32) { j := i32.add(j, 5:i32) }
		{
			//   for (i = 0; i < 5; i++)
			//     bc[i] = st[j + i];
			for { let i:i32 := 0:i32 } i32.lt_u(i, 5:i32) { i := i32.add(i, 1:i32) }
			{
				let bc_i:i32 := i32.add(i32.mul(8:i32, i), bc_offset(context_offset))
				let st_j_i:i32 := i32.add(i32.mul(8:i32, i32.add(j, i)), context_offset)
				i64.store(bc_i, i64.load(st_j_i))
			}
			//   for (i = 0; i < 5; i++)
			//     st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
			for { let i:i32 := 0:i32 } i32.lt_u(i, 5:i32) { i := i32.add(i, 1:i32) }
			{
				let st_j_i:i32 := i32.add(i32.mul(8:i32, i32.add(j, i)), context_offset)
				let li:i32 := i32.add(1:i32, i)
				let ri:i32 := i32.add(2:i32, i)
				if i32.ge_u(li, 5:i32) { li := 0:i32 }
				if i32.ge_u(ri, 5:i32) { ri := i32.sub(ri, 5:i32) }
				let bc_li:i32 := i32.add(i32.mul(8:i32, li), bc_offset(context_offset))
				let bc_ri:i32 := i32.add(i32.mul(8:i32, ri), bc_offset(context_offset))
				i64.store(st_j_i, i64.xor(i64.load(st_j_i), i64.and(bit_negate(i64.load(bc_li)), i64.load(bc_ri))))
			}
		}
		//  Iota
		//  st[0] ^= keccakf_rndc[r];
		let st_0:i32 := context_offset
		let keccakf_rndc_r:i32 := i32.add(rounds_consts_offset(context_offset), i32.mul(8:i32, r))
		i64.store(st_0, i64.xor(i64.load(st_0), i64.load(keccakf_rndc_r)))
	}
}
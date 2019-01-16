/**
 * @title Library to validate AZTEC zero-knowledge proofs
 * @author Zachary Williamson, AZTEC
 * @dev Don't include this as an internal library. This contract uses a static memory table to cache elliptic curve primitives and hashes.
 * Calling this internally from another function will lead to memory mutation and undefined behaviour.
 * The intended use case is to call this externally via `staticcall`. External calls to OptimizedAZTEC can be treated as pure functions as this contract contains no storage and makes no external calls (other than to precompiles)
 * Copyright Spilbury Holdings Ltd 2018. All rights reserved.
 * We will be releasing AZTEC as an open-source protocol that provides efficient transaction privacy for Ethereum.
 * This will include our bespoke AZTEC decentralized exchange, allowing for cross-asset transfers with full transaction privacy
 * and interopability with public decentralized exchanges.
 * Stay tuned for updates!
 *
 * Permission to use as test case in the Solidity compiler granted by the author:
 * https://github.com/ethereum/solidity/pull/5713#issuecomment-449042830
**/
{
    validateJoinSplit()
    // should not get here
    mstore(0x00, 404)
    revert(0x00, 0x20)


    function validateJoinSplit() {
        mstore(0x80, 7673901602397024137095011250362199966051872585513276903826533215767972925880) // h_x
        mstore(0xa0, 8489654445897228341090914135473290831551238522473825886865492707826370766375) // h_y
        let notes := add(0x04, calldataload(0x04))
        let m := calldataload(0x24)
        let n := calldataload(notes)
        let gen_order := 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001
        let challenge := mod(calldataload(0x44), gen_order)

        // validate m <= n
        if gt(m, n) { mstore(0x00, 404) revert(0x00, 0x20) }

        // recover k_{public} and calculate k_{public}
        let kn := calldataload(sub(calldatasize(), 0xc0))

        // add kn and m to final hash table
        mstore(0x2a0, caller())
        mstore(0x2c0, kn)
        mstore(0x2e0, m)
        kn := mulmod(sub(gen_order, kn), challenge, gen_order) // we actually want c*k_{public}
        hashCommitments(notes, n)
        let b := add(0x300, mul(n, 0x80))

        // Iterate over every note and calculate the blinding factor B_i = \gamma_i^{kBar}h^{aBar}\sigma_i^{-c}.
        // We use the AZTEC protocol pairing optimization to reduce the number of pairing comparisons to 1, which adds some minor alterations
        for { let i := 0 } lt(i, n) { i := add(i, 0x01) } {

            // Get the calldata index of this note
            let noteIndex := add(add(notes, 0x20), mul(i, 0xc0))


            let k
            let a := calldataload(add(noteIndex, 0x20))
            let c := challenge

            switch eq(add(i, 0x01), n)
            case 1 {
                k := kn

                // if all notes are input notes, invert k
                if eq(m, n) {
                    k := sub(gen_order, k)
                }
            }
            case 0 { k := calldataload(noteIndex) }

            // Check this commitment is well formed...
            validateCommitment(noteIndex, k, a)

            // If i > m then this is an output note.
            // Set k = kx_j, a = ax_j, c = cx_j, where j = i - (m+1)
            switch gt(add(i, 0x01), m)
            case 1 {

                // before we update k, update kn = \sum_{i=0}^{m-1}k_i - \sum_{i=m}^{n-1}k_i
                kn := addmod(kn, sub(gen_order, k), gen_order)
                let x := mod(mload(0x00), gen_order)
                k := mulmod(k, x, gen_order)
                a := mulmod(a, x, gen_order)
                c := mulmod(challenge, x, gen_order)

                // calculate x_{j+1}
                mstore(0x00, keccak256(0x00, 0x20))
            }
            case 0 {

                // nothing to do here except update kn = \sum_{i=0}^{m-1}k_i - \sum_{i=m}^{n-1}k_i
                kn := addmod(kn, k, gen_order)
            }

            calldatacopy(0xe0, add(noteIndex, 0x80), 0x40)
            calldatacopy(0x20, add(noteIndex, 0x40), 0x40)
            mstore(0x120, sub(gen_order, c))
            mstore(0x60, k)
            mstore(0xc0, a)

            // Using call instead of staticcall here to make it work on all targets.
            let result := call(gas(), 7, 0, 0xe0, 0x60, 0x1a0, 0x40)
            result := and(result, call(gas(), 7, 0, 0x20, 0x60, 0x120, 0x40))
            result := and(result, call(gas(), 7, 0, 0x80, 0x60, 0x160, 0x40))

            result := and(result, call(gas(), 6, 0, 0x120, 0x80, 0x160, 0x40))

            result := and(result, call(gas(), 6, 0, 0x160, 0x80, b, 0x40))

            if eq(i, m) {
                mstore(0x260, mload(0x20))
                mstore(0x280, mload(0x40))
                mstore(0x1e0, mload(0xe0))
                mstore(0x200, sub(0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd47, mload(0x100)))
            }

            if gt(i, m) {
                mstore(0x60, c)
                result := and(result, call(gas(), 7, 0, 0x20, 0x60, 0x220, 0x40))

                result := and(result, call(gas(), 6, 0, 0x220, 0x80, 0x260, 0x40))
                result := and(result, call(gas(), 6, 0, 0x1a0, 0x80, 0x1e0, 0x40))
            }

            if iszero(result) { mstore(0x00, 400) revert(0x00, 0x20) }
            b := add(b, 0x40) // increase B pointer by 2 words
        }

        if lt(m, n) {
            validatePairing(0x64)
        }

        let expected := mod(keccak256(0x2a0, sub(b, 0x2a0)), gen_order)
        if iszero(eq(expected, challenge)) {

            // No! Bad! No soup for you!
            mstore(0x00, 404)
            revert(0x00, 0x20)
        }

        // Great! All done. This is a valid proof so return ```true```
        mstore(0x00, 0x01)
        return(0x00, 0x20)
    }

    function validatePairing(t2) {
        let field_order := 0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd47
        let t2_x_1 := calldataload(t2)
        let t2_x_2 := calldataload(add(t2, 0x20))
        let t2_y_1 := calldataload(add(t2, 0x40))
        let t2_y_2 := calldataload(add(t2, 0x60))

        // check provided setup pubkey is not zero or g2
        if or(or(or(or(or(or(or(
            iszero(t2_x_1),
            iszero(t2_x_2)),
            iszero(t2_y_1)),
            iszero(t2_y_2)),
            eq(t2_x_1, 0x1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed)),
            eq(t2_x_2, 0x198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2)),
            eq(t2_y_1, 0x12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa)),
            eq(t2_y_2, 0x90689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b))
        {
            mstore(0x00, 400)
            revert(0x00, 0x20)
        }

        mstore(0x20, mload(0x1e0)) // sigma accumulator x
        mstore(0x40, mload(0x200)) // sigma accumulator y
        mstore(0x80, 0x1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed)
        mstore(0x60, 0x198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2)
        mstore(0xc0, 0x12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa)
        mstore(0xa0, 0x90689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b)
        mstore(0xe0, mload(0x260)) // gamma accumulator x
        mstore(0x100, mload(0x280)) // gamma accumulator y
        mstore(0x140, t2_x_1)
        mstore(0x120, t2_x_2)
        mstore(0x180, t2_y_1)
        mstore(0x160, t2_y_2)

        let success := call(gas(), 8, 0, 0x20, 0x180, 0x20, 0x20)

        if or(iszero(success), iszero(mload(0x20))) {
            mstore(0x00, 400)
            revert(0x00, 0x20)
        }
    }

    function validateCommitment(note, k, a) {
        let gen_order := 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001
        let field_order := 0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd47
        let gammaX := calldataload(add(note, 0x40))
        let gammaY := calldataload(add(note, 0x60))
        let sigmaX := calldataload(add(note, 0x80))
        let sigmaY := calldataload(add(note, 0xa0))
        if iszero(
            and(
                and(
                    and(
                        eq(mod(a, gen_order), a), // a is modulo generator order?
                        gt(a, 1)                  // can't be 0 or 1 either!
                    ),
                    and(
                        eq(mod(k, gen_order), k), // k is modulo generator order?
                        gt(k, 1)                  // and not 0 or 1
                    )
                ),
                and(
                    eq( // y^2 ?= x^3 + 3
                        addmod(mulmod(mulmod(sigmaX, sigmaX, field_order), sigmaX, field_order), 3, field_order),
                        mulmod(sigmaY, sigmaY, field_order)
                    ),
                    eq( // y^2 ?= x^3 + 3
                        addmod(mulmod(mulmod(gammaX, gammaX, field_order), gammaX, field_order), 3, field_order),
                        mulmod(gammaY, gammaY, field_order)
                    )
                )
            )
        ) {
            mstore(0x00, 400)
            revert(0x00, 0x20)
        }
    }

    function hashCommitments(notes, n) {
        for { let i := 0 } lt(i, n) { i := add(i, 0x01) } {
            let index := add(add(notes, mul(i, 0xc0)), 0x60)
            calldatacopy(add(0x300, mul(i, 0x80)), index, 0x80)
        }
        mstore(0x00, keccak256(0x300, mul(n, 0x80)))
    }
}
// ----
// fullSuite
// {
//     let validateJo__6 := 0x80
//     mstore(validateJo__6, 7673901602397024137095011250362199966051872585513276903826533215767972925880)
//     mstore(0xa0, 8489654445897228341090914135473290831551238522473825886865492707826370766375)
//     let validateJo__10 := calldataload(0x04)
//     let validateJo_notes := add(0x04, validateJo__10)
//     let validateJo_m := calldataload(0x24)
//     let validateJo_n := calldataload(validateJo_notes)
//     let validateJo_gen_order := 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001
//     let validateJo_challenge := mod(calldataload(0x44), validateJo_gen_order)
//     if gt(validateJo_m, validateJo_n)
//     {
//         mstore(0x00, 404)
//         revert(0x00, 0x20)
//     }
//     let validateJo_kn_287 := calldataload(add(calldatasize(), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff40))
//     let validateJo_kn := validateJo_kn_287
//     let validateJo__24 := 0x2a0
//     mstore(validateJo__24, caller())
//     mstore(0x2c0, validateJo_kn_287)
//     mstore(0x2e0, validateJo_m)
//     validateJo_kn := mulmod(sub(validateJo_gen_order, validateJo_kn_287), validateJo_challenge, validateJo_gen_order)
//     hashCommitments(validateJo_notes, validateJo_n)
//     let validateJo_b := add(0x300, mul(validateJo_n, validateJo__6))
//     let validateJo_i_290 := 0
//     let validateJo_i := validateJo_i_290
//     for {
//     }
//     lt(validateJo_i, validateJo_n)
//     {
//         validateJo_i := add(validateJo_i, 0x01)
//     }
//     {
//         let validateJo__34 := 0x20
//         let validateJo__351 := add(validateJo__10, mul(validateJo_i, 0xc0))
//         let validateJo_noteIndex := add(validateJo__351, 0x24)
//         let validateJo_k := validateJo_i_290
//         let validateJo_a_292 := calldataload(add(validateJo__351, 0x44))
//         let validateJo_a := validateJo_a_292
//         let validateJo_c := validateJo_challenge
//         let validateJo__39 := add(validateJo_i, 0x01)
//         switch eq(validateJo__39, validateJo_n)
//         case 1 {
//             validateJo_k := validateJo_kn
//             if eq(validateJo_m, validateJo_n)
//             {
//                 validateJo_k := sub(validateJo_gen_order, validateJo_kn)
//             }
//         }
//         case 0 {
//             validateJo_k := calldataload(validateJo_noteIndex)
//         }
//         validateCommitment(validateJo_noteIndex, validateJo_k, validateJo_a_292)
//         switch gt(validateJo__39, validateJo_m)
//         case 1 {
//             validateJo_kn := addmod(validateJo_kn, sub(validateJo_gen_order, validateJo_k), validateJo_gen_order)
//             let validateJo_x := mod(mload(validateJo_i_290), validateJo_gen_order)
//             validateJo_k := mulmod(validateJo_k, validateJo_x, validateJo_gen_order)
//             validateJo_a := mulmod(validateJo_a_292, validateJo_x, validateJo_gen_order)
//             validateJo_c := mulmod(validateJo_challenge, validateJo_x, validateJo_gen_order)
//             mstore(validateJo_i_290, keccak256(validateJo_i_290, validateJo__34))
//         }
//         case 0 {
//             validateJo_kn := addmod(validateJo_kn, validateJo_k, validateJo_gen_order)
//         }
//         let validateJo__52 := 0x40
//         calldatacopy(0xe0, add(validateJo__351, 164), validateJo__52)
//         calldatacopy(validateJo__34, add(validateJo__351, 100), validateJo__52)
//         let validateJo__61 := 0x120
//         mstore(validateJo__61, sub(validateJo_gen_order, validateJo_c))
//         let validateJo__62 := 0x60
//         mstore(validateJo__62, validateJo_k)
//         mstore(0xc0, validateJo_a)
//         let validateJo__65 := 0x1a0
//         let validateJo_result_302 := call(gas(), 7, validateJo_i_290, 0xe0, validateJo__62, validateJo__65, validateJo__52)
//         let validateJo_result := validateJo_result_302
//         let validateJo_result_303 := and(validateJo_result_302, call(gas(), 7, validateJo_i_290, validateJo__34, validateJo__62, validateJo__61, validateJo__52))
//         let validateJo__80 := 0x160
//         let validateJo_result_304 := and(validateJo_result_303, call(gas(), 7, validateJo_i_290, validateJo__6, validateJo__62, validateJo__80, validateJo__52))
//         let validateJo_result_305 := and(validateJo_result_304, call(gas(), 6, validateJo_i_290, validateJo__61, validateJo__6, validateJo__80, validateJo__52))
//         validateJo_result := and(validateJo_result_305, call(gas(), 6, validateJo_i_290, validateJo__80, validateJo__6, validateJo_b, validateJo__52))
//         if eq(validateJo_i, validateJo_m)
//         {
//             mstore(0x260, mload(validateJo__34))
//             mstore(0x280, mload(validateJo__52))
//             mstore(0x1e0, mload(0xe0))
//             mstore(0x200, sub(0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd47, mload(0x100)))
//         }
//         if gt(validateJo_i, validateJo_m)
//         {
//             mstore(validateJo__62, validateJo_c)
//             let validateJo__120 := 0x220
//             let validateJo_result_307 := and(validateJo_result, call(gas(), 7, validateJo_i_290, validateJo__34, validateJo__62, validateJo__120, validateJo__52))
//             let validateJo_result_308 := and(validateJo_result_307, call(gas(), 6, validateJo_i_290, validateJo__120, validateJo__6, 0x260, validateJo__52))
//             validateJo_result := and(validateJo_result_308, call(gas(), 6, validateJo_i_290, validateJo__65, validateJo__6, 0x1e0, validateJo__52))
//         }
//         if iszero(validateJo_result)
//         {
//             mstore(validateJo_i_290, 400)
//             revert(validateJo_i_290, validateJo__34)
//         }
//         validateJo_b := add(validateJo_b, validateJo__52)
//     }
//     if lt(validateJo_m, validateJo_n)
//     {
//         validatePairing(0x64)
//     }
//     if iszero(eq(mod(keccak256(validateJo__24, add(validateJo_b, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd60)), validateJo_gen_order), validateJo_challenge))
//     {
//         mstore(validateJo_i_290, 404)
//         revert(validateJo_i_290, 0x20)
//     }
//     mstore(validateJo_i_290, 0x01)
//     return(validateJo_i_290, 0x20)
//     mstore(validateJo_i_290, 404)
//     revert(validateJo_i_290, 0x20)
//     function validatePairing(t2)
//     {
//         let t2_x_1 := calldataload(t2)
//         let _165 := 0x20
//         let t2_x_2 := calldataload(add(t2, _165))
//         let t2_y_1 := calldataload(add(t2, 0x40))
//         let t2_y_2 := calldataload(add(t2, 0x60))
//         let _171 := 0x90689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b
//         let _173 := 0x12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa
//         let _175 := 0x198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2
//         let _177 := 0x1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed
//         if or(or(or(or(or(or(or(iszero(t2_x_1), iszero(t2_x_2)), iszero(t2_y_1)), iszero(t2_y_2)), eq(t2_x_1, _177)), eq(t2_x_2, _175)), eq(t2_y_1, _173)), eq(t2_y_2, _171))
//         {
//             mstore(0x00, 400)
//             revert(0x00, _165)
//         }
//         mstore(_165, mload(0x1e0))
//         mstore(0x40, mload(0x200))
//         mstore(0x80, _177)
//         mstore(0x60, _175)
//         mstore(0xc0, _173)
//         mstore(0xa0, _171)
//         mstore(0xe0, mload(0x260))
//         mstore(0x100, mload(0x280))
//         mstore(0x140, t2_x_1)
//         mstore(0x120, t2_x_2)
//         let _216 := 0x180
//         mstore(_216, t2_y_1)
//         mstore(0x160, t2_y_2)
//         let success := call(gas(), 8, 0, _165, _216, _165, _165)
//         if or(iszero(success), iszero(mload(_165)))
//         {
//             mstore(0, 400)
//             revert(0, _165)
//         }
//     }
//     function validateCommitment(note, k_1, a_2)
//     {
//         let gen_order_3 := 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001
//         let field_order_4 := 0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd47
//         let gammaX := calldataload(add(note, 0x40))
//         let gammaY := calldataload(add(note, 0x60))
//         let sigmaX := calldataload(add(note, 0x80))
//         let sigmaY := calldataload(add(note, 0xa0))
//         if iszero(and(and(and(eq(mod(a_2, gen_order_3), a_2), gt(a_2, 1)), and(eq(mod(k_1, gen_order_3), k_1), gt(k_1, 1))), and(eq(addmod(mulmod(mulmod(sigmaX, sigmaX, field_order_4), sigmaX, field_order_4), 3, field_order_4), mulmod(sigmaY, sigmaY, field_order_4)), eq(addmod(mulmod(mulmod(gammaX, gammaX, field_order_4), gammaX, field_order_4), 3, field_order_4), mulmod(gammaY, gammaY, field_order_4)))))
//         {
//             mstore(0x00, 400)
//             revert(0x00, 0x20)
//         }
//     }
//     function hashCommitments(notes_5, n_6)
//     {
//         let i_7 := 0
//         for {
//         }
//         lt(i_7, n_6)
//         {
//             i_7 := add(i_7, 0x01)
//         }
//         {
//             calldatacopy(add(0x300, mul(i_7, 0x80)), add(add(notes_5, mul(i_7, 0xc0)), 0x60), 0x80)
//         }
//         mstore(0, keccak256(0x300, mul(n_6, 0x80)))
//     }
// }

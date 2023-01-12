// This file is MIT Licensed.
//
// Copyright 2017 Christian Reitwiessner
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
pragma solidity ^0.8.0;
library Pairing {
    struct G1Point {
        uint X;
        uint Y;
    }
    // Encoding of field elements is: X[0] * z + X[1]
    struct G2Point {
        uint[2] X;
        uint[2] Y;
    }
    /// @return the generator of G1
    function P1() pure internal returns (G1Point memory) {
        return G1Point(1, 2);
    }
    /// @return the generator of G2
    function P2() pure internal returns (G2Point memory) {
        return G2Point(
            [10857046999023057135944570762232829481370756359578518086990519993285655852781,
             11559732032986387107991004021392285783925812861821192530917403151452391805634],
            [8495653923123431417604973247489272438418190587263600148770280649306958101930,
             4082367875863433681332203403145435568316851327593401208105741076214120093531]
        );
    }
    /// @return the negation of p, i.e. p.addition(p.negate()) should be zero.
    function negate(G1Point memory p) pure internal returns (G1Point memory) {
        // The prime q in the base field F_q for G1
        uint q = 21888242871839275222246405745257275088696311157297823662689037894645226208583;
        if (p.X == 0 && p.Y == 0)
            return G1Point(0, 0);
        return G1Point(p.X, q - (p.Y % q));
    }
    /// @return r the sum of two points of G1
    function addition(G1Point memory p1, G1Point memory p2) internal view returns (G1Point memory r) {
        uint[4] memory input;
        input[0] = p1.X;
        input[1] = p1.Y;
        input[2] = p2.X;
        input[3] = p2.Y;
        bool success;
        assembly {
            success := staticcall(sub(gas(), 2000), 6, input, 0xc0, r, 0x60)
            // Use "invalid" to make gas estimation work
            switch success case 0 { invalid() }
        }
        require(success);
    }


    /// @return r the product of a point on G1 and a scalar, i.e.
    /// p == p.scalar_mul(1) and p.addition(p) == p.scalar_mul(2) for all points p.
    function scalar_mul(G1Point memory p, uint s) internal view returns (G1Point memory r) {
        uint[3] memory input;
        input[0] = p.X;
        input[1] = p.Y;
        input[2] = s;
        bool success;
        assembly {
            success := staticcall(sub(gas(), 2000), 7, input, 0x80, r, 0x60)
            // Use "invalid" to make gas estimation work
            switch success case 0 { invalid() }
        }
        require (success);
    }
    /// @return the result of computing the pairing check
    /// e(p1[0], p2[0]) *  .... * e(p1[n], p2[n]) == 1
    /// For example pairing([P1(), P1().negate()], [P2(), P2()]) should
    /// return true.
    function pairing(G1Point[] memory p1, G2Point[] memory p2) internal view returns (bool) {
        require(p1.length == p2.length);
        uint elements = p1.length;
        uint inputSize = elements * 6;
        uint[] memory input = new uint[](inputSize);
        for (uint i = 0; i < elements; i++)
        {
            input[i * 6 + 0] = p1[i].X;
            input[i * 6 + 1] = p1[i].Y;
            input[i * 6 + 2] = p2[i].X[1];
            input[i * 6 + 3] = p2[i].X[0];
            input[i * 6 + 4] = p2[i].Y[1];
            input[i * 6 + 5] = p2[i].Y[0];
        }
        uint[1] memory out;
        bool success;
        assembly {
            success := staticcall(sub(gas(), 2000), 8, add(input, 0x20), mul(inputSize, 0x20), out, 0x20)
            // Use "invalid" to make gas estimation work
            switch success case 0 { invalid() }
        }
        require(success);
        return out[0] != 0;
    }
    /// Convenience method for a pairing check for two pairs.
    function pairingProd2(G1Point memory a1, G2Point memory a2, G1Point memory b1, G2Point memory b2) internal view returns (bool) {
        G1Point[] memory p1 = new G1Point[](2);
        G2Point[] memory p2 = new G2Point[](2);
        p1[0] = a1;
        p1[1] = b1;
        p2[0] = a2;
        p2[1] = b2;
        return pairing(p1, p2);
    }
    /// Convenience method for a pairing check for three pairs.
    function pairingProd3(
            G1Point memory a1, G2Point memory a2,
            G1Point memory b1, G2Point memory b2,
            G1Point memory c1, G2Point memory c2
    ) internal view returns (bool) {
        G1Point[] memory p1 = new G1Point[](3);
        G2Point[] memory p2 = new G2Point[](3);
        p1[0] = a1;
        p1[1] = b1;
        p1[2] = c1;
        p2[0] = a2;
        p2[1] = b2;
        p2[2] = c2;
        return pairing(p1, p2);
    }
    /// Convenience method for a pairing check for four pairs.
    function pairingProd4(
            G1Point memory a1, G2Point memory a2,
            G1Point memory b1, G2Point memory b2,
            G1Point memory c1, G2Point memory c2,
            G1Point memory d1, G2Point memory d2
    ) internal view returns (bool) {
        G1Point[] memory p1 = new G1Point[](4);
        G2Point[] memory p2 = new G2Point[](4);
        p1[0] = a1;
        p1[1] = b1;
        p1[2] = c1;
        p1[3] = d1;
        p2[0] = a2;
        p2[1] = b2;
        p2[2] = c2;
        p2[3] = d2;
        return pairing(p1, p2);
    }
}

contract Verifier {
    using Pairing for *;
    struct VerifyingKey {
        Pairing.G1Point alpha;
        Pairing.G2Point beta;
        Pairing.G2Point gamma;
        Pairing.G2Point delta;
        Pairing.G1Point[] gamma_abc;
    }
    struct Proof {
        Pairing.G1Point a;
        Pairing.G2Point b;
        Pairing.G1Point c;
    }
    function verifyingKey() pure internal returns (VerifyingKey memory vk) {
        vk.alpha = Pairing.G1Point(uint256(0x1e19a8a58ad52243374aeded373b7e89656ea339b9fa8ace98dd5fb221885ea2), uint256(0x2e66a9a67f1a9060a51da039c91c3402d1f46b71bbf10c7348ac4f13c3906736));
        vk.beta = Pairing.G2Point([uint256(0x1ca3e556290187c64a1057061f419a078dc71353f6af1066c03d7e1448bbc119), uint256(0x2bbc1b80e59743b489ec811b4ebf30a1ff540c2c37ced63d360b94f92f0a41fb)], [uint256(0x07eceb98d2fb10fa7363b45f51aa3d3ef3d511b482790645039a2562e2070f30), uint256(0x1c3e076d2aaf914abd6a49b72c4205669d3d1cbe4a4bf97b9ee49ac0fbbdbda9)]);
        vk.gamma = Pairing.G2Point([uint256(0x222c0019521d3e52881431be17cacaf8a7379398dd0833f60a2ac45f1c3fcd42), uint256(0x1018dbb94cd920bd55af4e2b12a9f740c6b38748a163b5dbd37c5ef6cf74708f)], [uint256(0x18bf34dc86b549a92f316f7a32070a3ce45a0f38fa45dda1162c4b6498baf286), uint256(0x12848d5a670b6102d5bd45d2b8250d50361001ea335ff6a1405a52504c22b8ac)]);
        vk.delta = Pairing.G2Point([uint256(0x13b8e16c40a6a299ea42107a97f881f9fa89986dcd5234ecb6919caf756ac1cb), uint256(0x25b64e4978690cd7cb531dbab0119148c96f5fc0c984c0cafb290bb75f033a09)], [uint256(0x1758eaa970929deff5e96e5852d21790c32591dbb13bc63e3df046f0271479a4), uint256(0x14d0b4222ad1710c6330e4bd8ad8f0d7b8f4cff0a37793d53001800e49f41192)]);
        vk.gamma_abc = new Pairing.G1Point[](9);
        vk.gamma_abc[0] = Pairing.G1Point(uint256(0x0925dc800d3a577859439a049f8ed0ae7a37dcd36652de478d662c08907a7626), uint256(0x1f7f76e299220ebf3da17bb415d25e6574e142391972dbd1513cf81341975cb5));
        vk.gamma_abc[1] = Pairing.G1Point(uint256(0x0e67dddfd91adad72376c56cbd98d5cfa4df5217d6115ff26ec741d0154f0bd8), uint256(0x0b97b2ddfdf4c31916d98e384bee3b24bfa0fc59a21ab489153f4dcd1a9a48ca));
        vk.gamma_abc[2] = Pairing.G1Point(uint256(0x1416b354665883cbbc5f5541012d1f8dd87ebb4415b3ee431be0804fff290bbf), uint256(0x1a284dd2eff43e6cb5aaea43dd9bee022ef0c91d90d0803cf5f7e4677e94a271));
        vk.gamma_abc[3] = Pairing.G1Point(uint256(0x2526852e7f009b4afa1fe0e1d30334c6e516fac223866b81a830b472164bdfe7), uint256(0x28adf6fbe54ba40afa91555c18477ad3a2f0a460f68d55a15b4e0c264b9c11c2));
        vk.gamma_abc[4] = Pairing.G1Point(uint256(0x22feae4a12bfb751638cd76b2373e84884ba4adef575ea14ed50c5954d31d410), uint256(0x108c3da0ffd7eda1fe7789e41693146beb979bd1644b19bbd517742ca3841348));
        vk.gamma_abc[5] = Pairing.G1Point(uint256(0x081477a5c52f41533cf6ca4f778ab922d59ba44b5a5e3fbdbf34ed8dc1a47a8d), uint256(0x0f2624780bd75b9f6c47f7bee582d02f1f983529b8aa9493ca848e38f2ec8447));
        vk.gamma_abc[6] = Pairing.G1Point(uint256(0x0603e7413c605d1e9b9352a62f0208e2bbd247d3cf3b3721f72c3a9407d679b9), uint256(0x0ce2d2dca8ae14ac4fd2f3fd89e602cdb45de815cf3ba183a25a47d877d9f6f5));
        vk.gamma_abc[7] = Pairing.G1Point(uint256(0x05bbe6b58285021fb843123971f8e2cfdd207b02c0aef5923ffe7ac841ee0cc9), uint256(0x2da1d3c2049546a7b46aaf89a8a7de493470087ce9af8ba37673f5ee8c35eb1b));
        vk.gamma_abc[8] = Pairing.G1Point(uint256(0x2c721270df9ba8884d309140f3a4b150a8e53a6c9d09bd8fc7c9aa3c4901aa8d), uint256(0x0de2cb1684759e693e855711fa1c381ae737e463447c3817df507a02064b470f));
    }
    function verify(uint[] memory input, Proof memory proof) internal view returns (uint) {
        uint256 snark_scalar_field = 21888242871839275222246405745257275088548364400416034343698204186575808495617;
        VerifyingKey memory vk = verifyingKey();
        require(input.length + 1 == vk.gamma_abc.length);
        // Compute the linear combination vk_x
        Pairing.G1Point memory vk_x = Pairing.G1Point(0, 0);
        for (uint i = 0; i < input.length; i++) {
            require(input[i] < snark_scalar_field);
            vk_x = Pairing.addition(vk_x, Pairing.scalar_mul(vk.gamma_abc[i + 1], input[i]));
        }
        vk_x = Pairing.addition(vk_x, vk.gamma_abc[0]);
        if(!Pairing.pairingProd4(
             proof.a, proof.b,
             Pairing.negate(vk_x), vk.gamma,
             Pairing.negate(proof.c), vk.delta,
             Pairing.negate(vk.alpha), vk.beta)) return 1;
        return 0;
    }
    function verifyTx(
            Proof memory proof, uint[8] memory input
        ) public view returns (bool r) {
        uint[] memory inputValues = new uint[](8);

        for(uint i = 0; i < input.length; i++){
            inputValues[i] = input[i];
        }
        if (verify(inputValues, proof) == 0) {
            return true;
        } else {
            return false;
        }
    }
}

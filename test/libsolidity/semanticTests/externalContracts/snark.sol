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
	function P1() internal returns (G1Point memory) {
		return G1Point(1, 2);
	}

	/// @return the generator of G2
	function P2() internal returns (G2Point memory) {
		return G2Point(
			[11559732032986387107991004021392285783925812861821192530917403151452391805634,
			 10857046999023057135944570762232829481370756359578518086990519993285655852781],
			[4082367875863433681332203403145435568316851327593401208105741076214120093531,
			 8495653923123431417604973247489272438418190587263600148770280649306958101930]
		);
	}

	/// @return the negation of p, i.e. p.add(p.negate()) should be zero.
	function negate(G1Point memory p) internal returns (G1Point memory) {
		// The prime q in the base field F_q for G1
		uint q = 21888242871839275222246405745257275088696311157297823662689037894645226208583;
		if (p.X == 0 && p.Y == 0)
			return G1Point(0, 0);
		return G1Point(p.X, q - (p.Y % q));
	}

	/// @return r the sum of two points of G1
	function add(G1Point memory p1, G1Point memory p2) internal returns (G1Point memory r) {
		uint[4] memory input;
		input[0] = p1.X;
		input[1] = p1.Y;
		input[2] = p2.X;
		input[3] = p2.Y;
		bool success;
		assembly {
			success := call(sub(gas(), 2000), 6, 0, input, 0xc0, r, 0x60)
			// Use "invalid" to make gas estimation work
			switch success case 0 { invalid() }
		}
		require(success);
	}

	/// @return r the product of a point on G1 and a scalar, i.e.
	/// p == p.mul(1) and p.add(p) == p.mul(2) for all points p.
	function mul(G1Point memory p, uint s) internal returns (G1Point memory r) {
		uint[3] memory input;
		input[0] = p.X;
		input[1] = p.Y;
		input[2] = s;
		bool success;
		assembly {
			success := call(sub(gas(), 2000), 7, 0, input, 0x80, r, 0x60)
			// Use "invalid" to make gas estimation work
			switch success case 0 { invalid() }
		}
		require(success);
	}

	/// @return the result of computing the pairing check
	/// e(p1[0], p2[0]) *  .... * e(p1[n], p2[n]) == 1
	/// For example pairing([P1(), P1().negate()], [P2(), P2()]) should
	/// return true.
	function pairing(G1Point[] memory p1, G2Point[] memory p2) internal returns (bool) {
		require(p1.length == p2.length);
		uint elements = p1.length;
		uint inputSize = p1.length * 6;
		uint[] memory input = new uint[](inputSize);
		for (uint i = 0; i < elements; i++)
		{
			input[i * 6 + 0] = p1[i].X;
			input[i * 6 + 1] = p1[i].Y;
			input[i * 6 + 2] = p2[i].X[0];
			input[i * 6 + 3] = p2[i].X[1];
			input[i * 6 + 4] = p2[i].Y[0];
			input[i * 6 + 5] = p2[i].Y[1];
		}
		uint[1] memory out;
		bool success;
		assembly {
			success := call(sub(gas(), 2000), 8, 0, add(input, 0x20), mul(inputSize, 0x20), out, 0x20)
			// Use "invalid" to make gas estimation work
			switch success case 0 { invalid() }
		}
		require(success);
		return out[0] != 0;
	}
	function pairingProd2(G1Point memory a1, G2Point memory a2, G1Point memory b1, G2Point memory b2) internal returns (bool) {
		G1Point[] memory p1 = new G1Point[](2);
		G2Point[] memory p2 = new G2Point[](2);
		p1[0] = a1;
		p1[1] = b1;
		p2[0] = a2;
		p2[1] = b2;
		return pairing(p1, p2);
	}
	function pairingProd3(
		G1Point memory a1, G2Point memory a2,
		G1Point memory b1, G2Point memory b2,
		G1Point memory c1, G2Point memory c2
	) internal returns (bool) {
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
	function pairingProd4(
		G1Point memory a1, G2Point memory a2,
		G1Point memory b1, G2Point memory b2,
		G1Point memory c1, G2Point memory c2,
			G1Point memory d1, G2Point memory d2
	) internal returns (bool) {
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

contract Test {
	using Pairing for *;
	struct VerifyingKey {
		Pairing.G2Point A;
		Pairing.G1Point B;
		Pairing.G2Point C;
		Pairing.G2Point gamma;
		Pairing.G1Point gammaBeta1;
		Pairing.G2Point gammaBeta2;
		Pairing.G2Point Z;
		Pairing.G1Point[] IC;
	}
	struct Proof {
		Pairing.G1Point A;
		Pairing.G1Point A_p;
		Pairing.G2Point B;
		Pairing.G1Point B_p;
		Pairing.G1Point C;
		Pairing.G1Point C_p;
		Pairing.G1Point K;
		Pairing.G1Point H;
	}
	function f() public returns (bool) {
		Pairing.G1Point memory p1;
		Pairing.G1Point memory p2;
		p1.X = 1; p1.Y = 2;
		p2.X = 1; p2.Y = 2;
		Pairing.G1Point memory explict_sum = Pairing.add(p1, p2);
		Pairing.G1Point memory scalar_prod = Pairing.mul(p1, 2);
		return (explict_sum.X == scalar_prod.X &&
			explict_sum.Y == scalar_prod.Y);
	}
	function g() public returns (bool) {
		Pairing.G1Point memory x = Pairing.add(Pairing.P1(), Pairing.negate(Pairing.P1()));
		// should be zero
		return (x.X == 0 && x.Y == 0);
	}
	function testMul() public returns (bool) {
		Pairing.G1Point memory p;
		// @TODO The points here are reported to be not well-formed
		p.X = 14125296762497065001182820090155008161146766663259912659363835465243039841726;
		p.Y = 16229134936871442251132173501211935676986397196799085184804749187146857848057;
		p = Pairing.mul(p, 13986731495506593864492662381614386532349950841221768152838255933892789078521);
		return
			p.X == 18256332256630856740336504687838346961237861778318632856900758565550522381207 &&
			p.Y == 6976682127058094634733239494758371323697222088503263230319702770853579280803;
	}
	function pair() public returns (bool) {
		Pairing.G2Point memory fiveTimesP2 = Pairing.G2Point(
			[4540444681147253467785307942530223364530218361853237193970751657229138047649, 20954117799226682825035885491234530437475518021362091509513177301640194298072],
			[11631839690097995216017572651900167465857396346217730511548857041925508482915, 21508930868448350162258892668132814424284302804699005394342512102884055673846]
		);
		// The prime p in the base field F_p for G1
		uint p = 21888242871839275222246405745257275088696311157297823662689037894645226208583;
		Pairing.G1Point[] memory g1points = new Pairing.G1Point[](2);
		Pairing.G2Point[] memory g2points = new Pairing.G2Point[](2);
		// check e(5 P1, P2)e(-P1, 5 P2) == 1
		g1points[0] = Pairing.P1().mul(5);
		g1points[1] = Pairing.P1().negate();
		g2points[0] = Pairing.P2();
		g2points[1] = fiveTimesP2;
		if (!Pairing.pairing(g1points, g2points))
			return false;
		// check e(P1, P2)e(-P1, P2) == 1
		g1points[0] = Pairing.P1();
		g1points[1] = Pairing.P1();
		g1points[1].Y = p - g1points[1].Y;
		g2points[0] = Pairing.P2();
		g2points[1] = Pairing.P2();
		if (!Pairing.pairing(g1points, g2points))
			return false;
		return true;
	}
	function verifyingKey() internal returns (VerifyingKey memory vk) {
		vk.A = Pairing.G2Point([0x209dd15ebff5d46c4bd888e51a93cf99a7329636c63514396b4a452003a35bf7, 0x04bf11ca01483bfa8b34b43561848d28905960114c8ac04049af4b6315a41678], [0x2bb8324af6cfc93537a2ad1a445cfd0ca2a71acd7ac41fadbf933c2a51be344d, 0x120a2a4cf30c1bf9845f20c6fe39e07ea2cce61f0c9bb048165fe5e4de877550]);
		vk.B = Pairing.G1Point(0x2eca0c7238bf16e83e7a1e6c5d49540685ff51380f309842a98561558019fc02, 0x03d3260361bb8451de5ff5ecd17f010ff22f5c31cdf184e9020b06fa5997db84);
		vk.C = Pairing.G2Point([0x2e89718ad33c8bed92e210e81d1853435399a271913a6520736a4729cf0d51eb, 0x01a9e2ffa2e92599b68e44de5bcf354fa2642bd4f26b259daa6f7ce3ed57aeb3], [0x14a9a87b789a58af499b314e13c3d65bede56c07ea2d418d6874857b70763713, 0x178fb49a2d6cd347dc58973ff49613a20757d0fcc22079f9abd10c3baee24590]);
		vk.gamma = Pairing.G2Point([0x25f83c8b6ab9de74e7da488ef02645c5a16a6652c3c71a15dc37fe3a5dcb7cb1, 0x22acdedd6308e3bb230d226d16a105295f523a8a02bfc5e8bd2da135ac4c245d], [0x065bbad92e7c4e31bf3757f1fe7362a63fbfee50e7dc68da116e67d600d9bf68, 0x06d302580dc0661002994e7cd3a7f224e7ddc27802777486bf80f40e4ca3cfdb]);
		vk.gammaBeta1 = Pairing.G1Point(0x15794ab061441e51d01e94640b7e3084a07e02c78cf3103c542bc5b298669f21, 0x14db745c6780e9df549864cec19c2daf4531f6ec0c89cc1c7436cc4d8d300c6d);
		vk.gammaBeta2 = Pairing.G2Point([0x1f39e4e4afc4bc74790a4a028aff2c3d2538731fb755edefd8cb48d6ea589b5e, 0x283f150794b6736f670d6a1033f9b46c6f5204f50813eb85c8dc4b59db1c5d39], [0x140d97ee4d2b36d99bc49974d18ecca3e7ad51011956051b464d9e27d46cc25e, 0x0764bb98575bd466d32db7b15f582b2d5c452b36aa394b789366e5e3ca5aabd4]);
		vk.Z = Pairing.G2Point([0x217cee0a9ad79a4493b5253e2e4e3a39fc2df38419f230d341f60cb064a0ac29, 0x0a3d76f140db8418ba512272381446eb73958670f00cf46f1d9e64cba057b53c], [0x26f64a8ec70387a13e41430ed3ee4a7db2059cc5fc13c067194bcc0cb49a9855, 0x2fd72bd9edb657346127da132e5b82ab908f5816c826acb499e22f2412d1a2d7]);
		vk.IC = new Pairing.G1Point[](10);
		vk.IC[0] = Pairing.G1Point(0x0aee46a7ea6e80a3675026dfa84019deee2a2dedb1bbe11d7fe124cb3efb4b5a, 0x044747b6e9176e13ede3a4dfd0d33ccca6321b9acd23bf3683a60adc0366ebaf);
		vk.IC[1] = Pairing.G1Point(0x1e39e9f0f91fa7ff8047ffd90de08785777fe61c0e3434e728fce4cf35047ddc, 0x2e0b64d75ebfa86d7f8f8e08abbe2e7ae6e0a1c0b34d028f19fa56e9450527cb);
		vk.IC[2] = Pairing.G1Point(0x1c36e713d4d54e3a9644dffca1fc524be4868f66572516025a61ca542539d43f, 0x042dcc4525b82dfb242b09cb21909d5c22643dcdbe98c4d082cc2877e96b24db);
		vk.IC[3] = Pairing.G1Point(0x17d5d09b4146424bff7e6fb01487c477bbfcd0cdbbc92d5d6457aae0b6717cc5, 0x02b5636903efbf46db9235bbe74045d21c138897fda32e079040db1a16c1a7a1);
		vk.IC[4] = Pairing.G1Point(0x0f103f14a584d4203c27c26155b2c955f8dfa816980b24ba824e1972d6486a5d, 0x0c4165133b9f5be17c804203af781bcf168da7386620479f9b885ecbcd27b17b);
		vk.IC[5] = Pairing.G1Point(0x232063b584fb76c8d07995bee3a38fa7565405f3549c6a918ddaa90ab971e7f8, 0x2ac9b135a81d96425c92d02296322ad56ffb16299633233e4880f95aafa7fda7);
		vk.IC[6] = Pairing.G1Point(0x09b54f111d3b2d1b2fe1ae9669b3db3d7bf93b70f00647e65c849275de6dc7fe, 0x18b2e77c63a3e400d6d1f1fbc6e1a1167bbca603d34d03edea231eb0ab7b14b4);
		vk.IC[7] = Pairing.G1Point(0x0c54b42137b67cc268cbb53ac62b00ecead23984092b494a88befe58445a244a, 0x18e3723d37fae9262d58b548a0575f59d9c3266db7afb4d5739555837f6b8b3e);
		vk.IC[8] = Pairing.G1Point(0x0a6de0e2240aa253f46ce0da883b61976e3588146e01c9d8976548c145fe6e4a, 0x04fbaa3a4aed4bb77f30ebb07a3ec1c7d77a7f2edd75636babfeff97b1ea686e);
		vk.IC[9] = Pairing.G1Point(0x111e2e2a5f8828f80ddad08f9f74db56dac1cc16c1cb278036f79a84cf7a116f, 0x1d7d62e192b219b9808faa906c5ced871788f6339e8d91b83ac1343e20a16b30);
	}
	function verify(uint[] memory input, Proof memory proof) internal returns (uint) {
		VerifyingKey memory vk = verifyingKey();
		require(input.length + 1 == vk.IC.length);
		// Compute the linear combination vk_x
		Pairing.G1Point memory vk_x = Pairing.G1Point(0, 0);
		for (uint i = 0; i < input.length; i++)
			vk_x = Pairing.add(vk_x, Pairing.mul(vk.IC[i + 1], input[i]));
		vk_x = Pairing.add(vk_x, vk.IC[0]);
		if (!Pairing.pairingProd2(proof.A, vk.A, Pairing.negate(proof.A_p), Pairing.P2())) return 1;
		if (!Pairing.pairingProd2(vk.B, proof.B, Pairing.negate(proof.B_p), Pairing.P2())) return 2;
		if (!Pairing.pairingProd2(proof.C, vk.C, Pairing.negate(proof.C_p), Pairing.P2())) return 3;
		if (!Pairing.pairingProd3(
			proof.K, vk.gamma,
			Pairing.negate(Pairing.add(vk_x, Pairing.add(proof.A, proof.C))), vk.gammaBeta2,
			Pairing.negate(vk.gammaBeta1), proof.B
		)) return 4;
		if (!Pairing.pairingProd3(
			Pairing.add(vk_x, proof.A), proof.B,
			Pairing.negate(proof.H), vk.Z,
			Pairing.negate(proof.C), Pairing.P2()
		)) return 5;
		return 0;
	}
	event Verified(string);
	function verifyTx() public returns (bool) {
		uint[] memory input = new uint[](9);
		Proof memory proof;
		proof.A = Pairing.G1Point(12873740738727497448187997291915224677121726020054032516825496230827252793177, 21804419174137094775122804775419507726154084057848719988004616848382402162497);
		proof.A_p = Pairing.G1Point(7742452358972543465462254569134860944739929848367563713587808717088650354556, 7324522103398787664095385319014038380128814213034709026832529060148225837366);
		proof.B = Pairing.G2Point(
			[8176651290984905087450403379100573157708110416512446269839297438960217797614, 15588556568726919713003060429893850972163943674590384915350025440408631945055],
			[15347511022514187557142999444367533883366476794364262773195059233657571533367, 4265071979090628150845437155927259896060451682253086069461962693761322642015]);
		proof.B_p = Pairing.G1Point(2979746655438963305714517285593753729335852012083057917022078236006592638393, 6470627481646078059765266161088786576504622012540639992486470834383274712950);
		proof.C = Pairing.G1Point(6851077925310461602867742977619883934042581405263014789956638244065803308498, 10336382210592135525880811046708757754106524561907815205241508542912494488506);
		proof.C_p = Pairing.G1Point(12491625890066296859584468664467427202390981822868257437245835716136010795448, 13818492518017455361318553880921248537817650587494176379915981090396574171686);
		proof.H = Pairing.G1Point(12091046215835229523641173286701717671667447745509192321596954139357866668225, 14446807589950902476683545679847436767890904443411534435294953056557941441758);
		proof.K = Pairing.G1Point(21341087976609916409401737322664290631992568431163400450267978471171152600502, 2942165230690572858696920423896381470344658299915828986338281196715687693170);
		input[0] = 13986731495506593864492662381614386532349950841221768152838255933892789078521;
		input[1] = 622860516154313070522697309645122400675542217310916019527100517240519630053;
		input[2] = 11094488463398718754251685950409355128550342438297986977413505294941943071569;
		input[3] = 6627643779954497813586310325594578844876646808666478625705401786271515864467;
		input[4] = 2957286918163151606545409668133310005545945782087581890025685458369200827463;
		input[5] = 1384290496819542862903939282897996566903332587607290986044945365745128311081;
		input[6] = 5613571677741714971687805233468747950848449704454346829971683826953541367271;
		input[7] = 9643208548031422463313148630985736896287522941726746581856185889848792022807;
		input[8] = 18066496933330839731877828156604;
		if (verify(input, proof) == 0) {
			emit Verified("Successfully verified.");
			return true;
		} else {
			return false;
		}
	}
}
/// Disabled because the point seems to be not well-formed, we need to find another example.
/// testMul() -> true
//
// ====
// EVMVersion: >=constantinople
// ----
// library: Pairing
// f() -> true
// g() -> true
// pair() -> true
// verifyTx() -> true
// ~ emit Verified(string): 0x20, 0x16, "Successfully verified."
// gas irOptimized: 95261
// gas legacy: 113953
// gas legacyOptimized: 83670

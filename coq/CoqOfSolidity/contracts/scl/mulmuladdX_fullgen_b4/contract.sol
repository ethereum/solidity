/********************************************************************************************/
/*
#/*   ╔═╗╔╦╗╔═╗╔═╗╔╦╗╦ ╦  ╔═╗╦═╗╦ ╦╔═╗╔╦╗╔═╗╦  ╦╔╗ 
#/*   ╚═╗║║║║ ║║ ║ ║ ╠═╣  ║  ╠╦╝╚╦╝╠═╝ ║ ║ ║║  ║╠╩╗
#/*   ╚═╝╩ ╩╚═╝╚═╝o╩ ╩ ╩  ╚═╝╩╚═ ╩ ╩   ╩ ╚═╝╩═╝╩╚═╝
#/*              
#/* Copyright (C) 2024 - Renaud Dubois - This file is part of SCL (Smoo.th CryptoLib) project
/* License: This software is licensed under MIT License                                        
/* 
/********************************************************************************************/
/* This file implements elliptic curve over short weierstrass form, with coefficient a=-3, with xyzz coordinates */
/* It is a custom 4 dimensional version of Shamir's trick (tis not a window)*/
/* (gen= any curve, sw=short weierstrass) */
/* b4=Four dimensional multiexponentiation */
// SPDX-License-Identifier: MIT
pragma solidity >=0.8.19 <0.9.0;


//error calling modExpPrecompile
uint256 constant _ModExpError=0x7FF;

//Starting from mload(0x40) this is the mapping in allocated memory
//https://medium.com/@ac1d_eth/technical-exploration-of-inline-assembly-in-solidity-b7d2b0b2bda8
//mapping from 0x40 in memory
uint256 constant _Prec_T8=0x800;
uint256 constant _Ap=0x820;
uint256 constant _y2=0x840;
uint256 constant _zzz2=0x860;
uint256 constant _free=0x880;

//mapping from Q in input to function, contains Qx, Qy, Qx', Qy', p, a, gx, gy, gx', gy'
//where P' is P multiplied by 2 pow 128 for shamir's multidimensional trick
//todo: remove all magic numbers
uint constant _Qx=0x00;
uint constant _Qy=0x20;
uint constant _Qx2pow128=0x40;
uint constant _Qy2pow128=0x60;
uint constant _modp=0x80;
uint constant _a=0xa0;
uint constant _gx=0xc0;
uint constant _gy=0xe0;
uint constant _gpow2p128_x=0x100;
uint constant _gpow2p128_y=0x120;


contract Contract {
    //this function is for use only after validation of the Q input:
    //Q shall belongs to the curve, and different from -P, -P128, -(P+P128), ...
    //those 16 values are tested by the ValidateKey function
    //due to handling of Neutral element, this function will not work for 16 specific weak keys
    //those value are excluded from the 
    function ecGenMulmuladdX_store(
        uint256 [10] memory Q,//store Qx, Qy, Q'x, Q'y p, a, gx, gy, gx2pow128, gy2pow128 
        uint256 scalar_u,
        uint256 scalar_v
    )   public view returns (uint256 X) {
        uint256 mask=1<<127;
        /* I. precomputations phase */

        if(scalar_u==0&&scalar_v==0){
            return 0;
        }
        uint256 Y;
        uint256 ZZZ;
        uint256 ZZ;
        
       // bytes memory Mem = new bytes(16*4*32);
        assembly ("memory-safe") {
        
         mstore(0x40, add(mload(0x40), _Prec_T8))
         mstore(add(mload(0x40), _Ap), mload(add(Q, _modp)))  //load modulus into AP addresse 

          //store 4 256 bits values starting from addr+offset
          function mstore4(addr, offset, val1, val2, val3, val4){
             mstore(add(offset, addr),val1 )
             offset:=add(32, offset)
             mstore(add(offset, addr),val2 )
             offset:=add(32, offset)
             mstore(add(offset, addr),val3 )
             offset:=add(32, offset)
             mstore(add(offset, addr),val4 )
             offset:=add(32, offset)
          }
          /* I. precomputations */
          //allocate memory for 15 projective points, first slot is unused
          {
           let _modulusp:=mload(add(mload(0x40), _Ap))   
         //normalized addition of two point, must not be neutral input 
         function ecAddn2(x1, y1, zz1, zzz1, x2, y2, _p) -> _x, _y, _zz, _zzz {
                y1 := sub(_p, y1)
                y2 := addmod(mulmod(y2, zzz1, _p), y1, _p)
                x2 := addmod(mulmod(x2, zz1, _p), sub(_p, x1), _p)
                _x := mulmod(x2, x2, _p) //PP = P^2
                _y := mulmod(_x, x2, _p) //PPP = P*PP
                _zz := mulmod(zz1, _x, _p) ////ZZ3 = ZZ1*PP
                
                _zzz := mulmod(zzz1, _y, _p) ////ZZZ3 = ZZZ1*PPP
                zz1 := mulmod(x1, _x, _p) //Q = X1*PP
                _x := addmod(addmod(mulmod(y2, y2, _p), sub(_p, _y), _p), mulmod(sub(_p,2), zz1, _p), _p) //R^2-PPP-2*Q

                x1:=mulmod(addmod(zz1, sub(_p, _x), _p), y2, _p)//necessary split not to explose stack
                _y := addmod(x1, mulmod(y1, _y, _p), _p) //R*(Q-X3)
           }

          mstore4(mload(0x40), 128, mload(add(Q,_gx)), mload(add(Q,_gy)), 1, 1)                       //G the base point
          mstore4(mload(0x40), 256, mload(add(Q,_gpow2p128_x)), mload(add(Q,_gpow2p128_y)), 1, 1)     //G'=2^128.G
          
          X:=mload(add(Q,_gpow2p128_x))
          Y:=mload(add(Q,_gpow2p128_y))
          X,Y,ZZ,ZZZ:=ecAddn2( X,Y,1,1, mload(add(Q,_gx)),mload(add(Q,_gy)), _modulusp) //G+G'
          mstore4(mload(0x40), 384, X,Y,ZZ,ZZZ)                        //Q, the public key
          mstore4(mload(0x40), 512, mload(Q),mload(add(32,Q)),1,1)
         
          X,Y,ZZ,ZZZ:=ecAddn2( mload(Q),mload(add(Q,32)),1,1, mload(add(Q,_gx)),mload(add(Q,_gy)),_modulusp )//G+Q
          mstore4(mload(0x40), 640, X,Y,ZZ,ZZZ)
         
          
          X:=mload(add(Q,_gpow2p128_x))
          Y:=mload(add(Q,_gpow2p128_y))
          X,Y,ZZ,ZZZ:=ecAddn2(X,Y,1,1,mload(Q),mload(add(Q,32)), _modulusp)//G'+Q
          mstore4(mload(0x40), 768, X,Y,ZZ,ZZZ)
        
          X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), _modulusp)//G'+Q+G
          mstore4(mload(0x40), 896, X,Y,ZZ,ZZZ)
         
          mstore4(mload(0x40), 1024, mload(add(Q, 64)), mload(add(Q, 96)),1,1)   //Q'=2^128.Q


          X,Y,ZZ,ZZZ:=ecAddn2(mload(add(Q, 64)), mload(add(Q, 96)),1,1, mload(add(Q,_gx)),mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q'+G
          mstore4(mload(0x40), 1152, X,Y,ZZ,ZZZ)
        
          
          X:=mload(add(Q,_gpow2p128_x))
          Y:=mload(add(Q,_gpow2p128_y))
          X,Y,ZZ,ZZZ:=ecAddn2(mload(add(Q, 64)), mload(add(Q, 96)),1,1, X,Y, mload(add(mload(0x40), _Ap))   )//Q'+G'
          mstore4(mload(0x40), 1280, X,Y,ZZ,ZZZ)
           
          X,Y,ZZ,ZZZ:=ecAddn2(X, Y, ZZ, ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q'+G'+G
          mstore4(mload(0x40), 1408, X,Y,ZZ,ZZZ)
           
          X,Y,ZZ,ZZZ:=ecAddn2( mload(Q),mload(add(Q,32)),1,1, mload(add(Q, 64)), mload(add(Q, 96)), mload(add(mload(0x40), _Ap))   )//Q+Q'
          mstore4(mload(0x40), 1536, X,Y,ZZ,ZZZ)

          X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ, mload(add(Q,_gx)), mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//Q+Q'+G
          mstore4(mload(0x40), 1664, X,Y,ZZ,ZZZ)

         X:= mload(add(768, mload(0x40)) )//G'+Q
         Y:= mload(add(800, mload(0x40)) )
         ZZ:= mload(add(832, mload(0x40)) )
         ZZZ:=mload(add(864, mload(0x40)) )
         X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ,mload(add(Q, 64)), mload(add(Q, 96)), mload(add(mload(0x40), _Ap))   )//G'+Q+Q'+
         mstore4(mload(0x40), 1792, X,Y,ZZ,ZZZ)

          X,Y,ZZ,ZZZ:=ecAddn2( X,Y,ZZ,ZZZ,mload(add(Q,0xc0)),mload(add(Q,_gy)), mload(add(mload(0x40), _Ap))   )//G'+Q+Q'+G
          //  Prec[15]
          mstore4(mload(0x40), 1920, X,Y,ZZ,ZZZ)
          }
        /*II. First MSB bit*/
                ZZZ:=0
                for {} iszero(ZZZ) { mask := shr(1, mask) }{
                ZZZ:=add(add(sub(1,iszero(and(scalar_u, mask))), shl(1,sub(1,iszero(and(shr(128, scalar_u), mask))))),
                           add(shl(2,sub(1,iszero(and(scalar_v, mask)))), shl(3,sub(1,iszero(and(shr(128, scalar_v), mask))))))

                }
                
              X:=mload(add(mload(0x40),shl(7,ZZZ)))//X
              Y:=mload(add(mload(0x40),add(32, shl(7,ZZZ))))//Y
              ZZ:=mload(add(mload(0x40),add(64, shl(7,ZZZ))))//ZZ
              ZZZ:=mload(add(mload(0x40),add(96, shl(7,ZZZ))))//ZZZ

                let Mem:=mload(0x40)
                let _p:=mload(add(Mem, _Ap))
        /*III. Main loop */
            //(X,Y,ZZ,ZZZ)=ec_Dbl(X,Y,ZZ,ZZZ);
            //TODO, replace mul by shifts
                for {} gt(mask, 0) { mask := shr(1, mask) } {
                    

                {
                //X,Y,ZZ,ZZZ:=ecDblNeg(X,Y,ZZ,ZZZ), not having it inplace increase by 12K the cost of the function

                let T1 := mulmod(2, Y, _p) //U = 2*Y1, y free
                let T2 := mulmod(T1, T1, _p) // V=U^2
                let T3 := mulmod(X, T2, _p) // S = X1*V
                T1 := mulmod(T1, T2, _p) // W=UV
                let T4:=mulmod(mload(add(Q,_a)),mulmod(ZZ,ZZ,_p),_p)

                T4 := addmod(mulmod(3, mulmod(X,X,_p),_p),T4,_p)//M=3*X12+aZZ12  
                ZZZ := mulmod(T1, ZZZ, _p) //zzz3=W*zzz1
                ZZ := mulmod(T2, ZZ, _p) //zz3=V*ZZ1
                X:=sub(_p,2)//-2
                X := addmod(mulmod(T4, T4, _p), mulmod(X, T3, _p), _p) //X3=M^2-2S
                T2 := mulmod(T4, addmod(X, sub(_p, T3), _p), _p) //-M(S-X3)=M(X3-S)
                Y := addmod(mulmod(T1, Y, _p), T2, _p) //-Y3= W*Y1-M(S-X3), we replace Y by -Y to avoid a sub in ecAdd
                //Y:=sub(p,Y)*/

                }

             //   let T4:=shl(128,mask)  
             // let T1:=add(add(sub(1,iszero(and(scalar_u, mask))), shl(1,sub(1,iszero(and(scalar_u, T4))))),
              //             add(shl(2,sub(1,iszero(and(scalar_v, mask)))), shl(3,sub(1,iszero(and(T4, scalar_v))))))
               
              let T1:=add(add(sub(1,iszero(and(scalar_u, mask))), shl(1,sub(1,iszero(and(shr(128, scalar_u), mask))))),
                           add(shl(2,sub(1,iszero(and(scalar_v, mask)))), shl(3,sub(1,iszero(and(shr(128, scalar_v), mask))))))
                            
              if iszero(T1) {
                            Y := sub(_p, Y)
                            continue
              }
              //inlined ec_Add
               T1:=shl(7, T1)//Commputed value address offset      
               
               let T4:=mload(add(Mem,T1))//X2
               mstore(add(Mem, _zzz2), mload(add(Mem,add(96,T1))))//ZZZ2

               if iszero(ZZ) {
                            X := T4//X2
                            Y := mload(add(Mem,add(32,T1)))//Y2
                            ZZ := mload(add(Mem,add(64,T1)))//ZZ2
                            ZZZ := mload(add(Mem,add(96,T1)))//ZZZ2
                            
                            continue
                        }  
                
                 mstore(add(Mem,_y2), addmod(mulmod( mload(add(Mem,add(32,T1))), ZZZ, _p), mulmod(Y,mload(add(Mem, _zzz2)), _p), _p))//R=S2-S1, sub avoided
                 T1:=mload(add(Mem,add(64,T1)))//zz2
                 let T2 := addmod(mulmod(T4, ZZ, _p), sub(_p, mulmod(X,T1,_p)), _p)//P=U2-U1

                        //special case ecAdd(P,P)=EcDbl
                        if iszero(mload(add(Mem,_y2))) {
                            if iszero(T2) {
                                T1 := mulmod(sub(_p,2), Y, _p) //U = 2*Y1, y free
                                T2 := mulmod(T1, T1, _p) // V=U^2
                                mstore(add(Mem,_y2), mulmod(X, T2, _p)) // S = X1*V

                                T1 := mulmod(T1, T2, _p) // W=UV
                                T4:=mulmod(mload(add(Q,_a)),mulmod(ZZ,ZZ,_p),_p)
                                T4 := addmod(mulmod(3, mulmod(X,X,_p),_p),T4,_p)//M=3*X12+aZZ12   //M

                                ZZZ := mulmod(T1, ZZZ, _p) //zzz3=W*zzz1
                                ZZ := mulmod(T2, ZZ, _p) //zz3=V*ZZ1, V free

                                X := addmod(mulmod(T4, T4, _p), mulmod(sub(_p,2), mload(add(Mem, _y2)), _p), _p) //X3=M^2-2S
                                T2 := mulmod(T4, addmod(mload(add(Mem, _y2)), sub(_p, X), _p), _p) //M(S-X3)

                                Y := addmod(T2, mulmod(T1, Y, _p), _p) //Y3= M(S-X3)-W*Y1
                               
                                continue
                            }
                        }
                  T4 := mulmod(T2, T2, _p) //PP
                  T2 := mulmod(T4, T2, _p) //PPP
                  ZZ := mulmod(mulmod(ZZ, T4,_p), T1 ,_p)//zz3=zz1*zz2*PP
                  T1:= mulmod(X,T1, _p)
                  ZZZ := mulmod(mulmod(ZZZ, T2, _p), mload(add(Mem, _zzz2)),_p) // zzz3=zzz1*zzz2*PPP
                  X := addmod(addmod(mulmod(mload(add(Mem, _y2)), mload(add(Mem, _y2)), _p), sub(_p, T2), _p), mulmod( T1 ,mulmod(sub(_p,2), T4, _p),_p ), _p)// R2-PPP-2*U1*PP
                  T4 := mulmod(T1, T4, _p)///Q=U1*PP
                  Y := addmod(mulmod(addmod(T4, sub(_p, X), _p), mload(add(Mem, _y2)), _p), mulmod(mulmod(Y,mload(add(Mem, _zzz2)), _p), T2, _p), _p)// R*(Q-X3)-S1*PPP

               }//endloop
                /* IV. Normalization */
                //(X,)=ec_Normalize(X,Y,ZZ,ZZZ);

                mstore(0x40, _free)
                let T := mload(0x40)
                mstore(add(T, 0x60), ZZ)
                //(X,Y)=ecZZ_SetAff(X,Y,zz, zzz);
                //T[0] = inverseModp_Hard(T[0], p); //1/zzz, inline modular inversion using Memmpile:
                // Define length of base, exponent and modulus. 0x20 == 32 bytes
                mstore(T, 0x20)
                mstore(add(T, 0x20), 0x20)
                mstore(add(T, 0x40), 0x20)
                // Define variables base, exponent and modulus
                //mstore(add(pointer, 0x60), u)
                mstore(add(T, 0x80), sub(_p,2))
                mstore(add(T, 0xa0), _p)

                // Call the precompiled contract 0x05 = ModExp
                if iszero(staticcall(not(0), 0x05, T, 0xc0, T, 0x20)) { 
                    mstore(0x40, _ModExpError)
                    revert(0x40, 0x20) }

                //Y:=mulmod(Y,zzz,p)//Y/zzz
                //zz :=mulmod(zz, mload(T),p) //1/z
                //zz:= mulmod(zz,zz,p) //1/zz
                X := mulmod(X, mload(T), _p) //X/zz   
          }//end assembly
    }
}

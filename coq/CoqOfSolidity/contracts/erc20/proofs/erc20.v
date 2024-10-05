Require Import CoqOfSolidity.CoqOfSolidity.
Require Import proofs.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.
Require simulations.erc20.
Require Import contracts.erc20.erc20.
Require Import contracts.erc20.erc20_shallow.

Import RunO.

Ltac Zify.zify_post_hook ::= Z.to_euclidean_division_equations.

Opaque StdlibAux.keccak256.

(** Direct proofs of elements of the `erc20.v` file. *)
Module erc20.
  Module Storage.
    Module Valid.
      Record t (s : erc20.Storage.t) : Prop := {
        balances : Dict.Valid.t Address.Valid.t U256.Valid.t s.(erc20.Storage.balances);
        allowances :
          Dict.Valid.t
            (fun '(owner, spender) => Address.Valid.t owner /\ Address.Valid.t spender)
            U256.Valid.t
            s.(erc20.Storage.allowances);
        total_supply : U256.Valid.t s.(erc20.Storage.total_supply);
      }.
    End Valid.

    Lemma init_is_valid : Valid.t erc20.Storage.init.
    Proof.
      constructor; hfcrush.
    Qed.
  End Storage.

  Lemma totalSupply_is_valid (s : erc20.Storage.t)
      (H_s : Storage.Valid.t s) :
    U256.Valid.t (erc20.totalSupply s).
  Proof.
    sauto lq: on.
  Qed.

  Lemma balanceOf_is_valid (s : erc20.Storage.t) (owner : Address.t)
      (H_s : Storage.Valid.t s)
      (H_owner : Address.Valid.t owner) :
    U256.Valid.t (erc20.balanceOf s owner).
  Proof.
    unfold erc20.balanceOf.
    destruct H_s.
    pose proof (Dict.get_is_valid _ _ _ owner balances).
    unfold U256.Valid.t in *.
    destruct Dict.get; lia.
  Qed.

  Lemma _transfer_is_valid (s s' : erc20.Storage.t) (from to : Address.t) (value : U256.t)
      (H_s : Storage.Valid.t s)
      (H_from : Address.Valid.t from)
      (H_to : Address.Valid.t to)
      (H_value : U256.Valid.t value)
      (H_s' : erc20._transfer from to value s = erc20.Result.Success s') :
    Storage.Valid.t s'.
  Proof.
  Admitted.
End erc20.

Module Erc20_403.
  (** This function will only be executed on zero. *)
  Lemma run_checked_add_uint256 codes environment state :
    {{? codes, environment, state |
      Erc20_403.checked_add_uint256 0 ⇓
      Result.Ok 20
    | state ?}}.
  Proof.
    unfold Erc20_403.checked_add_uint256.
    repeat (lu || cu || p).
  Qed.

  Lemma run_body codes environment state :
    environment.(Environment.callvalue) = 0 ->
    environment.(Environment.caller) <> 0 ->
    let storage' := [
      StorableValue.Map
        (Dict.declare_or_assign []
          environment.(Environment.caller)
          20
        );
      StorableValue.Map []; 
      StorableValue.U256 20
    ] in
    exists memory',
    {{? codes, environment,
        Some (make_state environment state SimulatedMemory.init SimulatedStorage.init) |
      Erc20_403.body ⇓
      Result.Return 128 32
    | Some (make_state environment state memory' storage') ?}}.
  Proof.
    eexists.
    unfold Erc20_403.body.
    l. {
      l. {
        c. {
          p.
        }
        p.
      }
      l. {
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          unfold Stdlib.callvalue.
          pr.
          p.
        }
        unfold M.if_unit.
        destruct (_ =? 0)%Z eqn:?; [|lia].
        p.
      }
      l. {
        c. {
          unfold Stdlib.caller.
          pr.
          p.
        }
        s.
        c. {
          p.
        }
        s.
        unfold Stdlib.Pure.iszero.
        destruct (_ =? 0)%Z eqn:?; [lia|].
        p.
      }
      l. {
        c. {
          apply_run_sload_u256.
        }
        s.
        c. {
          apply run_checked_add_uint256.
        }
        c. {
          apply_run_sstore_u256.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          unfold Stdlib.caller.
          pr.
          p.
        }
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          apply_run_keccak256_tuple2.
        }
        s.
        c. {
          apply_run_sload_map_u256.
        }
        c. {
          apply run_checked_add_uint256.
        }
        p.
      }
      l. {
        c. {
          unfold Stdlib.caller.
          pr.
          p.
        }
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          apply_run_keccak256_tuple2.
        }
        c. {
          apply_run_sstore_map_u256.
        }
        CanonizeState.execute.
        p.
      }
      s.
      l. {
        c. {
          apply_run_mload.
        }
        p.
      }
      s.
      l. {
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      l. {
        c. {
          unfold Stdlib.caller.
          pr.
          p.
        }
        c. {
          p.
        }
        p.
      }
      l. {
        c. {
          apply_run_mload.
        }
        p.
      }
      l. {
        c. {
          p.
        }
        p.
      }
      l. {
        c. {
          p.
        }
        c. {
          match goal with
          | |- context[Stdlib.codecopy _ ?value] =>
            let value' := eval cbv in value in
            change value with value'
          end.
          unfold Stdlib.codecopy.
          match goal with
          | |- context[if ?value =? 0 then _ else _] =>
            let value' := eval cbv in value in
            change value with value'
          end.
          s.
          match goal with
          | |- {{? _, _, Some (make_state _ _ ?memory _) | _ ⇓ _ | _ ?}} =>
            apply (Memory.run_mstore _ _ _ memory 4 _)
          end.
        }
        CanonizeState.execute.
        p.
      }
      s.
      l. {
        c. {
          p.
        }
        p.
      }
      p.
    }
    p.
  Qed.

  Module Erc20_403_deployed.
    Module SimulatedCalldata.
      Record t : Set := {
        header : Z;
        data : list U256.t;
      }.

      Definition to_calldata (calldata : t) : list U256.t.
      Admitted.
    End SimulatedCalldata.

    Lemma run_calldataload codes environment state
        (calldata : SimulatedCalldata.t) (offset : nat) (value : U256.t) :
      environment.(Environment.calldata) = SimulatedCalldata.to_calldata calldata ->
      List.nth_error calldata.(SimulatedCalldata.data) offset = Some value ->
      {{? codes, environment, state |
        Stdlib.calldataload (4 + 32 * Z.of_nat offset) ⇓
        Result.Ok value
      | state ?}}.
    Proof.
    Admitted.

    Ltac apply_run_calldataload :=
      match goal with
      | |- {{? _, _, _ | Stdlib.calldataload ?offset ⇓ _ | _ ?}} =>
        eapply (run_calldataload _ _ _ _ (Z.to_nat ((offset - 4) / 32)));
        match goal with
        | H : _ |- _ => apply H
        end
      end.

    Lemma run_abi_decode_address codes environment state :
      {{? codes, environment, Some state |
        Erc20_403.Erc20_403_deployed.abi_decode_address ⇓
        let calldata := environment.(Environment.calldata) in
        let address := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        if erc20.Payload.get_is_address_valid address then
          Result.Ok address
        else
          Result.Revert 0 0
      | Some state ?}}.
    Proof.
      unfold Erc20_403.Erc20_403_deployed.abi_decode_address.
      lu; c; [pr; p|]; s.
      lu; c; [p|]; s.
      repeat (c; [p|]; s).
      unfold M.if_unit, erc20.Payload.get_is_address_valid.
      unfold Stdlib.Pure.and, Stdlib.Pure.eq, Stdlib.Pure.sub, Stdlib.Pure.shl; s.
      set (address := StdlibAux.get_calldata_u256 _ _).
      destruct (_ =? _); s.
      { p. }
      { lu; cu; p. }
    Qed.

    Lemma run_abi_decode_address_2305 codes environment state :
      {{? codes, environment, Some state |
        Erc20_403.Erc20_403_deployed.abi_decode_address_2305 ⇓
        let calldata := environment.(Environment.calldata) in
        let address := StdlibAux.get_calldata_u256 calldata (4 + 32 * 1) in
        if erc20.Payload.get_is_address_valid address then
          Result.Ok address
        else
          Result.Revert 0 0
      | Some state ?}}.
    Proof.
      unfold Erc20_403.Erc20_403_deployed.abi_decode_address_2305.
      lu; c; [pr; p|]; s.
      lu; c; [p|]; s.
      repeat (c; [p|]; s).
      unfold M.if_unit, erc20.Payload.get_is_address_valid.
      unfold Stdlib.Pure.and, Stdlib.Pure.eq, Stdlib.Pure.sub, Stdlib.Pure.shl; s.
      set (address := StdlibAux.get_calldata_u256 _ _).
      destruct (_ =? _); s.
      { p. }
      { lu; cu; p. }
    Qed.

    Definition simulation_checked_sub_uint256 (x y : U256.t) : Result.t U256.t :=
      if x <? y then
        Result.Revert 0 0x24
      else
        Result.Ok (x - y).

    Lemma run_checked_sub_uint256 codes environment state
        (x y : U256.t) :
      U256.Valid.t x ->
      U256.Valid.t y ->
      let output := simulation_checked_sub_uint256 x y in
      {{? codes, environment, Some state |
        Erc20_403.Erc20_403_deployed.checked_sub_uint256 x y ⇓
        output
      | wrap_state_with_revert state output
      ?}}.
    Proof.
      intros.
      unfold Erc20_403.Erc20_403_deployed.checked_sub_uint256.
      unfold U256.Valid.t in *.
      l. {
        c. {
          p.
        }
        p.
      }
      s.
      l. {
        c. {
          p.
        }
        s.
        unfold M.if_unit.
        apply RunO.If; [p|].
        l. {
          c. {
            p.
          }
          c. {
            prn; intros [].
            p.
          }
          p.
        }
        l. {
          c. {
            prn; intros [].
            p.
          }
          p.
        }
        l. {
          c. {
            p.
          }
          p.
        }
        p.
      }
      unfold output, simulation_checked_sub_uint256.
      unfold Stdlib.Pure.gt, Stdlib.Pure.sub.
      destruct (_ >? _) eqn:?, (_ <? _) eqn:?; s; cbn in * |-; try lia.
      { p. }
      { pe; trivial.
        f_equal; lia.
      }
    Qed.

    Definition simulation_checked_add_uint256 (x y : U256.t) : Result.t U256.t :=
      if x + y >=? 2 ^ 256 then
        Result.Revert 0 0x24
      else
        Result.Ok (x + y).

    Lemma run_checked_add_uint256 codes environment state
        (x y : U256.t) :
      U256.Valid.t x ->
      U256.Valid.t y ->
      let output := simulation_checked_add_uint256 x y in
      {{? codes, environment, Some state |
        Erc20_403.Erc20_403_deployed.checked_add_uint256 x y ⇓
        output
      | wrap_state_with_revert state output
      ?}}.
    Proof.
      intros.
      unfold Erc20_403.Erc20_403_deployed.checked_add_uint256.
      unfold U256.Valid.t in *.
      l. {
        c. {
          p.
        }
        p.
      }
      s.
      l. {
        c. {
          p.
        }
        s.
        unfold M.if_unit.
        apply RunO.If; [p|].
        l. {
          c. {
            p.
          }
          c. {
            prn; intros [].
            p.
          }
          p.
        }
        s.
        l. {
          c. {
            prn; intros [].
            p.
          }
          p.
        }
        l. {
          c. {
            p.
          }
          p.
        }
        p.
      }
      unfold output, simulation_checked_add_uint256.
      unfold Stdlib.Pure.gt, Stdlib.Pure.add.
      destruct (_ >? _) eqn:?, (_ >=? _) eqn:?; s; cbn in * |-; try lia.
      { p. }
      { pe; trivial.
        f_equal; lia.
       }
    Qed.

    Module SimulatedStorage.
      Definition of_erc20_state (state : erc20.Storage.t) : SimulatedStorage.t := [
        IsStorable.to_storable_value state.(erc20.Storage.balances);
        IsStorable.to_storable_value state.(erc20.Storage.allowances);
        StorableValue.U256 state.(erc20.Storage.total_supply)
      ].
    End SimulatedStorage.

    Definition extract_address (address : Address.t) : M.t U256.t :=
      [[ Stdlib.and ~(| address, (Stdlib.sub ~(| (Stdlib.shl ~(| 160, 1 |)), 1 |)) |) ]].

    Lemma run_is_non_null_address codes environment state
      (address : Address.t)
      (H_address : Address.Valid.t address) :
      {{? codes, environment, Some state |
        extract_address address ⇓
        Result.Ok address
      | Some state ?}}.
    Proof.
      repeat (
        c ||
        p
      ).
      rewrite <- Address.implies_and_mask by assumption.
      p.
    Qed.

    Lemma run_fun_approve codes environment state
        (owner spender : Address.t) (value : U256.t) (s : erc20.Storage.t)
        (mem_0 mem_1 mem_3 mem_4 : U256.t)
        (H_owner : Address.Valid.t owner)
        (H_spender : Address.Valid.t spender) :
      let memoryguard := 0x80 in
      let memory_start :=
        [mem_0; mem_1; memoryguard; mem_3; mem_4] in
      let state_start :=
        make_state environment state memory_start (SimulatedStorage.of_erc20_state s) in
      let output :=
        erc20._approve owner spender value s in
      let memory_end :=
        [spender; erc20.keccak256_tuple2 owner 1; memoryguard; mem_3; value] in
      let state_end :=
        match output with
        | erc20.Result.Revert _ _ => None
        | erc20.Result.Success s =>
          Some (make_state environment state memory_end (SimulatedStorage.of_erc20_state s))
        end in
      {{? codes, environment, Some state_start |
        Erc20_403.Erc20_403_deployed.fun_approve owner spender value ⇓
        match output with
        | erc20.Result.Revert p s => Result.Revert p s
        | erc20.Result.Success _ => Result.Ok tt
        end
      | state_end ?}}.
    Proof.
      simpl.
      unfold Erc20_403.Erc20_403_deployed.fun_approve, erc20._approve.
      l. {
        now apply run_is_non_null_address.
      }
      unfold Stdlib.Pure.iszero.
      lu.
      c; [p|].
      s.
      unfold Stdlib.Pure.iszero.
      destruct (owner =? 0); s. {
        change (true || _) with true; s.
        lu; c. {
          apply_run_mload.
        }
        repeat (
          lu ||
          cu ||
          (prn; intro) ||
          s ||
          p
        ).
      }
      l. {
        now apply run_is_non_null_address.
      }
      lu.
      c; [p|]; s.
      unfold Stdlib.Pure.iszero.
      change (false || ?e) with e; s.
      destruct (spender =? 0); s. {
        lu; c. {
          apply_run_mload.
        }
        repeat (
          lu ||
          cu ||
          (prn; intro) ||
          s ||
          p
        ).
      }
      lu; c. {
        apply_run_mstore.
      }
      CanonizeState.execute.
      lu; c. {
        apply_run_mstore.
      }
      CanonizeState.execute.
      lu; c. {
        apply_run_keccak256_tuple2.
      }
      lu.
      lu; c. {
        apply_run_mstore.
      }
      CanonizeState.execute.
      lu; c. {
        apply_run_mstore.
      }
      CanonizeState.execute.
      lu; c. {
        apply_run_keccak256_tuple2.
      }
      lu; c. {
        apply_run_sstore_map2_u256.
      }
      CanonizeState.execute.
      lu; c. {
        apply_run_mload.
      }
      s.
      lu; c. {
        apply_run_mstore.
      }
      CanonizeState.execute.
      lu; c. {
        p.
      }
      p.
    Qed.

    Ltac run_fun_transfer_line :=
      (lu; c; [
        apply_run_mload ||
        apply_run_mstore ||
        p
      |]) ||
      (lu; c; [
        apply_run_keccak256_tuple2 |
        c; [ apply_run_sstore_map_u256 |]
      ]) ||
      CanonizeState.execute.

    Lemma run_fun_transfer codes environment state
      (from to : Address.t) (value : U256.t) (s : erc20.Storage.t)
      (H_s : erc20.Storage.Valid.t s)
      (H_from : Address.Valid.t from)
      (H_to : Address.Valid.t to)
      (H_value : U256.Valid.t value) :
      let memoryguard := 0x80 in
      let memory_start :=
        [0; 0; memoryguard; 0; 0] in
      let state_start :=
        make_state environment state memory_start (SimulatedStorage.of_erc20_state s) in
      let output :=
        erc20._transfer from to value s in
      let memory_end :=
        [to; 0; memoryguard; 0; value] in
      let state_end :=
        match output with
        | erc20.Result.Revert _ _ => None
        | erc20.Result.Success s =>
          Some (make_state environment state memory_end (SimulatedStorage.of_erc20_state s))
        end in
      {{? codes, environment, Some state_start |
        Erc20_403.Erc20_403_deployed.fun_transfer from to value ⇓
        match output with
        | erc20.Result.Revert p s => Result.Revert p s
        | erc20.Result.Success _ => Result.Ok tt
        end
      | state_end ?}}.
    Proof.
      simpl.
      unfold Erc20_403.Erc20_403_deployed.fun_transfer, erc20._transfer.
      l. {
        now apply run_is_non_null_address.
      }
      unfold Stdlib.iszero, Stdlib.Pure.iszero.
      lu.
      c; [p|].
      destruct (to =? 0); s. {
        lu; c. {
          apply_run_mload.
        }
        repeat (
          lu ||
          cu ||
          (prn; intro) ||
          s ||
          p
        ).
      }
      l. {
        now apply run_is_non_null_address.
      }
      repeat run_fun_transfer_line.
      lu; c. {
        apply_run_keccak256_tuple2.
      }
      c. {
        apply_run_sload_map_u256.
      }
      s.
      c. {
        apply run_checked_sub_uint256; trivial.
        now apply erc20.balanceOf_is_valid.
      }
      unfold simulation_checked_sub_uint256.
      destruct (_ <? _) eqn:H_balance_from_eq. {
        p.
      }
      simpl.
      repeat run_fun_transfer_line.
      lu; c. {
        apply_run_keccak256_tuple2.
      }
      c. {
        apply_run_sload_map_u256.
      }
      CanonizeState.execute.
      s.
      c. {
        apply run_checked_add_uint256; trivial.
        destruct H_s.
        eapply StorableValue.map_get_u256_is_valid.
        apply Dict.declare_or_assign_is_valid;
          try apply balances;
          try assumption.
        change (StorableValue.map_get_u256 _ from) with (erc20.balanceOf s from).
        set (balance_from := erc20.balanceOf s from) in *.
        assert (U256.Valid.t balance_from). {
          apply erc20.balanceOf_is_valid; [constructor |]; assumption.
        }
        unfold U256.Valid.t in *.
        lia.
      }
      unfold simulation_checked_add_uint256.
      destruct (_ >=? _). {
        p.
      }
      simpl.
      repeat run_fun_transfer_line.
      p.
    Qed.

    Module Environment.
      Module Valid.
        Record t (environment : Environment.t) : Prop := {
          caller : Address.Valid.t environment.(Environment.caller);
          calldata : List.Forall (fun byte => 0 <= byte < 2 ^ 8) environment.(Environment.calldata);
        }.
      End Valid.
    End Environment.

    Ltac explicit_selector signature :=
      let selector := eval cbv in (get_selector signature) in
      change selector with (get_selector signature).

    Lemma run_body codes environment state
        (s : erc20.Storage.t)
        (H_environment : Environment.Valid.t environment)
        (H_s : erc20.Storage.Valid.t s) :
      let memoryguard := 128 in
      let memory_start :=
        [0; 0; 0; 0; 0] in
      let state_start :=
        make_state environment state memory_start (SimulatedStorage.of_erc20_state s) in
      let output :=
        erc20.body
          environment.(Environment.caller)
          environment.(Environment.callvalue)
          s
          environment.(Environment.calldata) in
      let memory_end_middle :=
        [memoryguard; 0] in
      let state_end :=
        match output with
        | erc20.Result.Revert _ _ => None
        | erc20.Result.Success (memory_end_beginning, memory_end_end, s) =>
          Some (make_state environment state
            (memory_end_beginning ++ memory_end_middle ++ memory_end_end)
            (SimulatedStorage.of_erc20_state s)
          )
        end in
      {{? codes, environment, Some state_start |
        Erc20_403.Erc20_403_deployed.body ⇓
        match output with
        | erc20.Result.Revert p s => Result.Revert p s
        | erc20.Result.Success (_, memory_end_end, _) =>
          Result.Return memoryguard (32 * Z.of_nat (List.length memory_end_end))
        end
      | state_end ?}}.
    Proof.
      simpl; unfold Erc20_403.Erc20_403_deployed.body, erc20.body.
      lu.
      l. {
        cu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        p.
      }
      lu.
      repeat (
        c ||
        pr ||
        p
      ).
      unfold erc20.Payload.of_calldata.
      unfold Stdlib.Pure.lt.
      destruct (_ <? 4). {
        s.
        repeat (lu || cu || p).
        pn.
      }
      lu.
      c. {
        unfold Stdlib.calldataload.
        pr; p.
      }
      s.
      cu.
      s.
      (* We can get the list of signatures using the `--hashes` option of `solc`. *)
      explicit_selector "allowance(address,address)".
      explicit_selector "approve(address,uint256)".
      explicit_selector "balanceOf(address)".
      explicit_selector "decreaseAllowance(address,uint256)".
      explicit_selector "increaseAllowance(address,uint256)".
      explicit_selector "totalSupply()".
      explicit_selector "transfer(address,uint256)".
      explicit_selector "transferFrom(address,address,uint256)".
      destruct (_ =? get_selector "approve(address,uint256)") eqn:?. {
        l. {
          c. {
            unfold Stdlib.callvalue.
            pr.
            p.
          }
          unfold M.if_unit.
          instantiate (2 := if environment.(Environment.callvalue) =? 0 then _ else _).
          destruct (_ =? 0).
          { p. }
          { lu; c; p. }
        }
        unfold erc20.main; s.
        destruct (environment.(Environment.callvalue) =? 0); s.
        2: {
          pn.
        }
        lu.
        c. { pr; p. }
        s.
        cu.
        cu.
        cu.
        unfold M.if_unit; s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: {
          l; [cu; pn|]; s.
          pn.
        }
        lu; c. {
          apply run_abi_decode_address.
        }
        s.
        set (spender := StdlibAux.get_calldata_u256 _ 4).
        destruct erc20.Payload.get_is_address_valid eqn:H_spender; s.
        2: pn.
        lu.
        c; [pr; p|].
        c; [pr; p|].
        s.
        c. {
          apply run_fun_approve;
            try apply H_environment.
          now apply erc20.Payload.get_is_address_valid_implies_valid.
        }
        unfold erc20.approve; s.
        destruct erc20._approve; s.
        2: pn.
        lu; c. {
          apply_run_mload.
        }
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        s.
        lu; cu.
        s.
        p.
      }
      destruct (_ =? get_selector"totalSupply()") eqn:?. {
        lu; cu; pr; s.
        unfold M.if_unit.
        destruct (_ =? 0); s.
        2: lu; cu; pn.
        lu.
        repeat (cu || p || pr); s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: lu; cu; pn.
        s.
        unfold erc20.totalSupply.
        lu; c. {
          apply_run_sload_u256.
        }
        lu; c. {
          apply_run_mload.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; cu; p.
      }
      destruct (_ =? get_selector "transferFrom(address,address,uint256)") eqn:?. {
        lu; cu; pr; s.
        unfold M.if_unit.
        destruct (_ =? 0); s.
        2: lu; cu; pn.
        lu.
        repeat (cu || p || pr); s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: lu; cu; pn.
        lu; c. {
          apply run_abi_decode_address.
        }
        s.
        set (from := StdlibAux.get_calldata_u256 _ 4).
        destruct erc20.Payload.get_is_address_valid eqn:H_from; s.
        2: pn.
        lu; c. {
          apply run_abi_decode_address_2305.
        }
        s.
        set (to := StdlibAux.get_calldata_u256 _ 36).
        destruct erc20.Payload.get_is_address_valid eqn:H_to in |- *; s.
        2: pn.
        lu; cu; pr; s.
        lu; c. {
          now apply run_fun_transfer;
            try apply erc20.Payload.get_is_address_valid_implies_valid;
            try apply StdlibAux.get_calldata_u256_is_valid;
            try apply H_environment.
        }
        unfold erc20.transferFrom.
        destruct erc20._transfer as [s'|] eqn:H_s'_eq; s.
        2: pn.
        l. {
          repeat (
            (c; [apply_run_mstore|]) ||
            cu
          ).
          CanonizeState.execute.
          unfold Stdlib.Pure.and, Stdlib.Pure.sub, Stdlib.Pure.shl.
          rewrite <- Address.implies_and_mask
            by now apply erc20.Payload.get_is_address_valid_implies_valid.
          p.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        s.
        lu; s.
        l. {
          repeat (
            (c; [apply_run_mstore|]) ||
            cu ||
            pr
          ).
          CanonizeState.execute.
          unfold Stdlib.Pure.and, Stdlib.Pure.sub, Stdlib.Pure.shl.
          rewrite <- Address.implies_and_mask by apply H_environment.
          p.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        s.
        lu; cu; pr; s.
        c. {
          apply_run_sload_map2_u256.
        }
        s.
        c. {
          apply run_checked_sub_uint256;
            try eapply StorableValue.map_get_u256_is_valid;
            try eapply (erc20._transfer_is_valid _ _ _ _ _ H_s);
            try apply H_s'_eq;
            try apply StdlibAux.get_calldata_u256_is_valid;
            try apply H_environment;
            try apply erc20.Payload.get_is_address_valid_implies_valid;
            assumption.
        }
        unfold erc20.getAllowance, simulation_checked_sub_uint256.
        set (val := StdlibAux.get_calldata_u256 _ 68).
        unfold StorableValue.map_get_u256.
        destruct (_ <? _); s.
        1: p.
        c. {
          apply run_fun_approve;
            try apply H_environment;
            try apply erc20.Payload.get_is_address_valid_implies_valid;
            assumption.
        }
        destruct erc20._approve; s.
        2: pn.
        lu; c. {
          apply_run_mload.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; cu; s.
        p.
      }
      destruct (_ =? get_selector "increaseAllowance(address,uint256)") eqn:?. {
        lu; cu; pr; s.
        unfold M.if_unit.
        destruct (_ =? 0); s.
        2: lu; cu; pn.
        lu.
        repeat (cu || p || pr); s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: lu; cu; pn.
        s.
        lu; c. {
          apply run_abi_decode_address.
        }
        s.
        set (to := StdlibAux.get_calldata_u256 _ 4).
        destruct erc20.Payload.get_is_address_valid eqn:H_to; s.
        2: pn.
        lu; cu; pr; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        s.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        lu; s.
        l. {
          repeat (
            (c; [apply_run_mstore|]) ||
            cu ||
            pr
          ).
          CanonizeState.execute.
          unfold Stdlib.Pure.and, Stdlib.Pure.sub, Stdlib.Pure.shl.
          rewrite <- Address.implies_and_mask
            by now apply erc20.Payload.get_is_address_valid_implies_valid.
          p.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        s.
        lu; cu; pr; s.
        c. {
          apply_run_sload_map2_u256.
        }
        cu; pr; s.
        set (val := StdlibAux.get_calldata_u256 _ 36).
        c. {
          apply run_checked_add_uint256;
            try apply StdlibAux.get_calldata_u256_is_valid;
            try eapply StorableValue.map_get_u256_is_valid;
            try apply H_s;
            try apply H_environment.
        }
        unfold simulation_checked_add_uint256,
          erc20.increaseAllowance,
          erc20.getAllowance,
          StorableValue.map_get_u256.
        destruct (_ >=? _); s.
        1: p.
        c. {
          apply run_fun_approve;
            try apply H_environment;
            try apply erc20.Payload.get_is_address_valid_implies_valid;
            assumption.
        }
        destruct erc20._approve; s.
        2: pn.
        lu; c. {
          apply_run_mload.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; cu; s.
        p.
      }
      destruct (_ =? get_selector "balanceOf(address)") eqn:?. {
        lu; cu; pr; s.
        unfold M.if_unit.
        destruct (_ =? 0); s.
        2: lu; cu; pn.
        lu.
        repeat (cu || p || pr); s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: lu; cu; pn.
        s.
        lu; c. {
          apply run_abi_decode_address.
        }
        s.
        set (owner := StdlibAux.get_calldata_u256 _ 4).
        destruct erc20.Payload.get_is_address_valid eqn:H_owner; s.
        2: pn.
        repeat (
          c; [apply_run_mstore|] ||
          cu
        ).
        CanonizeState.execute.
        rewrite <- Address.implies_and_mask by
          now apply erc20.Payload.get_is_address_valid_implies_valid.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        s.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        s.
        c. {
          apply_run_sload_map_u256.
        }
        lu; c. {
          apply_run_mload.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; cu; s.
        p.
      }
      destruct (_ =? get_selector "decreaseAllowance(address,uint256)") eqn:?. {
        lu; cu; pr; s.
        unfold M.if_unit.
        destruct (_ =? 0); s.
        2: lu; cu; pn.
        lu.
        repeat (cu || p || pr); s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: lu; cu; pn.
        lu; c. {
          apply run_abi_decode_address.
        }
        s.
        set (to := StdlibAux.get_calldata_u256 _ 4).
        destruct erc20.Payload.get_is_address_valid eqn:H_to; s.
        2: pn.
        lu; cu; pr.
        c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        s.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        lu; s.
        l. {
          repeat (
            (c; [apply_run_mstore|]) ||
            cu ||
            pr
          ).
          CanonizeState.execute.
          rewrite <- Address.implies_and_mask by
            now apply erc20.Payload.get_is_address_valid_implies_valid.
          p.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        s.
        lu; cu; pr; s.
        c. {
          apply_run_sload_map2_u256.
        }
        cu; pr; s.
        set (val := StdlibAux.get_calldata_u256 _ 36).
        c. {
          apply run_checked_sub_uint256;
            try eapply StorableValue.map_get_u256_is_valid;
            try apply H_s;
            try apply StdlibAux.get_calldata_u256_is_valid;
            try apply H_environment.
        }
        s.
        unfold simulation_checked_sub_uint256,
          erc20.decreaseAllowance,
          erc20.getAllowance,
          StorableValue.map_get_u256.
        destruct (_ <? _); s.
        1: p.
        c. {
          apply run_fun_approve;
            try apply H_environment;
            try apply erc20.Payload.get_is_address_valid_implies_valid;
            assumption.
        }
        destruct erc20._approve; s.
        2: pn.
        lu; c. {
          apply_run_mload.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; cu; s.
        p.
      }
      destruct (_ =? get_selector "transfer(address,uint256)") eqn:?. {
        lu; cu; pr; s.
        unfold M.if_unit.
        destruct (_ =? 0); s.
        2: lu; cu; pn.
        lu.
        repeat (cu || p || pr); s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: lu; cu; pn.
        lu; c. {
          apply run_abi_decode_address.
        }
        s.
        set (to := StdlibAux.get_calldata_u256 _ 4).
        destruct erc20.Payload.get_is_address_valid eqn:H_to; s.
        2: pn.
        lu; repeat (cu; pr);s.
        c. {
          apply run_fun_transfer; try assumption;
            try apply H_environment;
            try apply erc20.Payload.get_is_address_valid_implies_valid;
            try apply StdlibAux.get_calldata_u256_is_valid;
            try apply H_environment;
            assumption.
        }
        s; unfold erc20.transfer.
        destruct erc20._transfer; s.
        2: pn.
        lu; c. {
          apply_run_mload.
        }
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        s.
        lu; cu; s.
        p.
      }
      destruct (_ =? get_selector "allowance(address,address)") eqn:?. {
        lu; cu; pr; s.
        unfold M.if_unit.
        destruct (_ =? 0); s.
        2: lu; cu; pn.
        lu.
        repeat (cu || p || pr); s.
        rewrite <- erc20.Payload.get_have_enough_calldata_eq.
        destruct erc20.Payload.get_have_enough_calldata; s.
        2: lu; cu; pn.
        s.
        lu; c. {
          apply run_abi_decode_address.
        }
        s.
        set (owner := StdlibAux.get_calldata_u256 _ 4).
        destruct erc20.Payload.get_is_address_valid eqn:H_owner; s.
        2: pn.
        lu; c. {
          apply run_abi_decode_address_2305.
        }
        s.
        set (spender := StdlibAux.get_calldata_u256 _ 36).
        destruct erc20.Payload.get_is_address_valid eqn:H_spender in |- *; s.
        2: pn.
        l. {
          repeat (
            (c; [apply_run_mstore|]) ||
            cu
          ).
          CanonizeState.execute.
          rewrite <- Address.implies_and_mask by
            now apply erc20.Payload.get_is_address_valid_implies_valid.
          p.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        s.
        lu; s.
        l. {
          repeat (
            (c; [apply_run_mstore|]) ||
            cu ||
            pr
          ).
          CanonizeState.execute.
          rewrite <- Address.implies_and_mask by
            now apply erc20.Payload.get_is_address_valid_implies_valid.
          p.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; c. {
          apply_run_keccak256_tuple2.
        }
        s.
        lu; c. {
          apply_run_sload_map2_u256.
        }
        lu; c. {
          apply_run_mload.
        }
        s.
        lu; c. {
          apply_run_mstore.
        }
        CanonizeState.execute.
        lu; cu; s.
        p.
      }
      s.
      lu; cu; pn.
    Qed.
  End Erc20_403_deployed.
End Erc20_403.

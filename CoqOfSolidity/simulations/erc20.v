Require Import CoqOfSolidity.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.

Require Import test.libsolidity.semanticTests.various.erc20.ERC20.
Require Import test.libsolidity.semanticTests.various.erc20.GeneratedTest.

Ltac Zify.zify_post_hook ::= Z.to_euclidean_division_equations.

Definition normalize_i8 (n : Z) : Z :=
  ((n + 128) mod 256) - 128.

Compute normalize_i8 (-1025).

Definition opposite_i8 (n : Z) : Z :=
  normalize_i8 (-n).

Compute opposite_i8 0. (* 0 *)
Compute opposite_i8 40. (* -40 *)
Compute opposite_i8 (-28). (* 28 *)
Compute opposite_i8 (-128). (* -128 *)

Compute (opposite_i8 0).
Compute (opposite_i8 40).
Compute (opposite_i8 (-20)).
Compute (opposite_i8 (-128)).

Lemma normalize_i8_eq (n : Z) :
  - 127 <= n < 128 ->
  opposite_i8 n = - n.
Proof.
  unfold opposite_i8, normalize_i8.
  lia.
Qed.

Definition half := 2 ^ 63.
Definition full := 2 ^ 64.

Definition normalize (n : Z) : Z :=
  ((n + half) mod full) - half.

Compute normalize (-1025).

Definition opposite (n : Z) : Z :=
  normalize (-n).

Compute opposite 0. (* 0 *)
Compute opposite 40. (* -40 *)
Compute opposite (-28). (* 28 *)
Compute opposite (-128). (* -128 *)

Compute (opposite 0).
Compute (opposite 40).
Compute (opposite (-20)).
Compute (opposite (-128)).

Lemma normalize_eq (n : Z) :
  - half < n < half ->
  opposite n = - n.
Proof.
  unfold opposite, normalize, half, full.
  Time lia.
Qed.

Compute (opposite_i8 (-128)).
Compute (opposite_i8 (-129)).
Compute (opposite_i8 (127)).
Compute (opposite_i8 (128)).

Module Address.
  Definition t : Set :=
    U256.t.
End Address.

Module Storage.
  Record t := {
    balances : Dict.t Address.t U256.t;
    allowances : Dict.t (Address.t * Address.t) U256.t;
    total_supply : U256.t;
  }.

  Definition init : t := {|
    balances := [];
    allowances := [];
    total_supply := 0;
  |}.
End Storage.

Module Result.
  Inductive t (A : Set) : Set :=
  | Success (value : A)
  | Revert (p s : U256.t).
  Arguments Success {_}.
  Arguments Revert {_}.
End Result.

Definition constructor (sender : Address.t) : Storage.t :=
  {|
    Storage.balances := [(sender, 20)];
    Storage.allowances := [];
    Storage.total_supply := 20;
  |}.

Definition totalSupply (s : Storage.t) : U256.t :=
  s.(Storage.total_supply).

Definition balanceOf (s : Storage.t) (owner : Address.t) : U256.t :=
  match Dict.get s.(Storage.balances) owner with
  | Some balance => balance
  | None => 0
  end.

Definition getAllowance (s : Storage.t) (owner spender : Address.t) : U256.t :=
  match Dict.get s.(Storage.allowances) (owner, spender) with
  | Some allowance => allowance
  | None => 0
  end.

Definition revert_address_null {A : Set} : Result.t A :=
  Result.Revert 128 132.

Definition revert_arithmetic {A : Set} : Result.t A :=
  Result.Revert 0 36.

Definition _transfer (from to : Address.t) (value : U256.t) (s : Storage.t) : Result.t Storage.t :=
  if to =? 0 then
    revert_address_null
  else if balanceOf s from <? value then
    revert_arithmetic
  else
    let s :=
      s <| Storage.balances :=
        Dict.declare_or_assign s.(Storage.balances) from (balanceOf s from - value)
      |> in
    if balanceOf s to + value >=? 2 ^ 256 then
      revert_arithmetic
    else
      Result.Success s <| Storage.balances :=
        Dict.declare_or_assign s.(Storage.balances) to (balanceOf s to + value)
      |>.

Definition _mint (account : Address.t) (value : U256.t) (s : Storage.t) : option Storage.t :=
  if account =? 0 then
    None
  else
    if s.(Storage.total_supply) + value >=? 2 ^ 256 then
      None
    else
      let s := s <| Storage.total_supply := s.(Storage.total_supply) + value |> in
      if balanceOf s account + value >=? 2 ^ 256 then
        None
      else
        Some s <| Storage.balances :=
          Dict.declare_or_assign s.(Storage.balances) account (balanceOf s account + value)
        |>.

Definition _burn (account : Address.t) (value : U256.t) (s : Storage.t) : Result.t Storage.t :=
  if account =? 0 then
    revert_address_null
  else if value >=? s.(Storage.total_supply) then
    revert_arithmetic
  else
    let s := s <| Storage.total_supply := s.(Storage.total_supply) - value |> in
    if balanceOf s account <? value then
      revert_arithmetic
    else
      Result.Success s <| Storage.balances :=
        Dict.declare_or_assign s.(Storage.balances) account (balanceOf s account - value)
      |>.

Definition _approve (owner spender : Address.t) (value : U256.t) (s : Storage.t) : Result.t Storage.t :=
  if (owner =? 0) || (spender =? 0) then
    revert_address_null
  else
    Result.Success s <| Storage.allowances :=
      Dict.declare_or_assign s.(Storage.allowances) (owner, spender) value
    |>.

Definition _burnFrom (sender : Address.t) (account : Address.t) (value : U256.t) (s : Storage.t) :
    Result.t Storage.t :=
  match _burn account value s with
  | Result.Revert p s => Result.Revert p s
  | Result.Success s =>
    let allowance := getAllowance s account sender in
    if allowance <? value then
      revert_arithmetic
    else
      _approve account sender (allowance - value) s
  end.

Definition transfer (sender : U256.t) (s : Storage.t) (to : Address.t) (value : U256.t) :
    Result.t (bool * Storage.t) :=
  let s := _transfer sender to value s in
  match s with
  | Result.Success s => Result.Success (true, s)
  | Result.Revert p s => Result.Revert p s
  end.

Definition approve (sender : U256.t) (s : Storage.t) (spender : Address.t) (value : U256.t) :
    Result.t (bool * Storage.t) :=
  let s := _approve sender spender value s in
  match s with
  | Result.Success s => Result.Success (true, s)
  | Result.Revert p s => Result.Revert p s
  end.

Definition transferFrom (sender : U256.t) (from to : Address.t) (value : U256.t) (s : Storage.t) :
    Result.t (bool * Storage.t) :=
  let s := _transfer from to value s in
  match s with
  | Result.Revert p s => Result.Revert p s
  | Result.Success s =>
    if getAllowance s from sender <? value then
      revert_arithmetic
    else
      let s := _approve from sender (getAllowance s from sender - value) s in
      match s with
      | Result.Revert p s => Result.Revert p s
      | Result.Success s => Result.Success (true, s)
      end
  end.

Definition increaseAllowance (sender : U256.t) (spender : Address.t) (addedValue : U256.t)
    (s : Storage.t) : Result.t (bool * Storage.t) :=
  if getAllowance s sender spender + addedValue >=? 2 ^ 256 then
    revert_arithmetic
  else
    let s := _approve sender spender (getAllowance s sender spender + addedValue) s in
    match s with
    | Result.Success s => Result.Success (true, s)
    | Result.Revert p s => Result.Revert p s
    end.

Definition decreaseAllowance (sender : U256.t) (spender : Address.t) (subtractedValue : U256.t)
    (s : Storage.t) : Result.t (bool * Storage.t) :=
  if getAllowance s sender spender <? subtractedValue then
    revert_arithmetic
  else
    let s := _approve sender spender (getAllowance s sender spender - subtractedValue) s in
    match s with
    | Result.Success s => Result.Success (true, s)
    | Result.Revert p s => Result.Revert p s
    end.

Module Payload.
  Inductive t : Set :=
  | Transfer (to: Address.t) (value: U256.t)
  | Approve (spender: Address.t) (value: U256.t)
  | TransferFrom (from: Address.t) (to: Address.t) (value: U256.t)
  | IncreaseAllowance (spender: Address.t) (addedValue: U256.t)
  | DecreaseAllowance (spender: Address.t) (subtractedValue: U256.t)
  | TotalSupply
  | BalanceOf (owner: Address.t)
  | Allowance (owner: Address.t) (spender: Address.t).

  Definition to_calldata (payload: t) : Z * list U256.t :=
    match payload with
    | Transfer to value =>
      (0xa9059cbb, [to; value])
    | Approve spender value =>
      (0x095ea7b3, [spender; value])
    | TransferFrom from to value =>
      (0x23b872dd, [from; to; value])
    | IncreaseAllowance spender addedValue =>
      (0x39509351, [spender; addedValue])
    | DecreaseAllowance spender subtractedValue =>
      (0xa457c2d7, [spender; subtractedValue])
    | TotalSupply =>
      (0x18160ddd, [])
    | BalanceOf owner =>
      (0x70a08231, [owner])
    | Allowance owner spender =>
      (0xdd62ed3e, [owner; spender])
    end.

  Definition get_return_Set (payload : t) : Set :=
    match payload with
    | Transfer _ _ => bool
    | Approve _ _ => bool
    | TransferFrom _ _ _ => bool
    | IncreaseAllowance _ _ => bool
    | DecreaseAllowance _ _ => bool
    | TotalSupply => U256.t
    | BalanceOf _ => U256.t
    | Allowance _ _ => U256.t
    end.

  Definition serialize_return (payload : t) (value : get_return_Set payload) : list U256.t :=
    match payload, value with
    | Transfer _ _, b => [U256.of_bool b]
    | Approve _ _, b => [U256.of_bool b]
    | TransferFrom _ _ _, b => [U256.of_bool b]
    | IncreaseAllowance _ _, b => [U256.of_bool b]
    | DecreaseAllowance _ _, b => [U256.of_bool b]
    | TotalSupply, v => [v]
    | BalanceOf _, v => [v]
    | Allowance _ _, v => [v]
    end.

  Definition get_have_enough_calldata (minimum : Z) (calldata : list U256.t) : bool :=
    Z.of_nat (List.length calldata) >=? minimum + 4.

  Lemma get_have_enough_calldata_eq (minimum : Z) (calldata : list U256.t) :
    get_have_enough_calldata minimum calldata =
    (Stdlib.Pure.slt
      (Stdlib.Pure.add
        (Z.of_nat (Datatypes.length calldata))
        (Stdlib.Pure.not 3)) minimum =? 0).
  Proof.
  Admitted.

  Definition get_is_address_valid (address : U256.t) : bool :=
    address =? Z.land address (2 ^ 160 - 1).

  Lemma get_is_address_valid_implies_valid (address : U256.t) :
    get_is_address_valid address = true ->
    Address.Valid.t address.
  Proof.
  Admitted.

  Definition of_calldata (callvalue : U256.t) (calldata: list U256.t) : option t :=
    if Z.of_nat (List.length calldata) <? 4 then
      None
    else
      let selector := Stdlib.Pure.shr (256 - 32) (StdlibAux.get_calldata_u256 calldata 0) in
      if selector =? get_selector "approve(address,uint256)" then
        let to := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        let value := StdlibAux.get_calldata_u256 calldata (4 + 32 * 1) in
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 2) calldata) then
          None
        else if negb (get_is_address_valid to) then
          None
        else
          Some (Approve to value)
      else if selector =? get_selector "totalSupply()" then
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 0) calldata) then
          None
        else
          Some TotalSupply
      else if selector =? get_selector "transferFrom(address,address,uint256)" then
        let from := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        let to := StdlibAux.get_calldata_u256 calldata (4 + 32 * 1) in
        let value := StdlibAux.get_calldata_u256 calldata (4 + 32 * 2) in
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 3) calldata) then
          None
        else if negb (get_is_address_valid from) then
          None
        else if negb (get_is_address_valid to) then
          None
        else
          Some (TransferFrom from to value)
      else if selector =? get_selector "increaseAllowance(address,uint256)" then
        let to := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        let value := StdlibAux.get_calldata_u256 calldata (4 + 32 * 1) in
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 2) calldata) then
          None
        else if negb (get_is_address_valid to) then
          None
        else
          Some (IncreaseAllowance to value)
      else if selector =? get_selector "balanceOf(address)" then
        let owner := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 1) calldata) then
          None
        else if negb (get_is_address_valid owner) then
          None
        else
          Some (BalanceOf owner)
      else if selector =? get_selector "decreaseAllowance(address,uint256)" then
        let to := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        let value := StdlibAux.get_calldata_u256 calldata (4 + 32 * 1) in
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 2) calldata) then
          None
        else if negb (get_is_address_valid to) then
          None
        else
          Some (DecreaseAllowance to value)
      else if selector =? get_selector "transfer(address,uint256)" then
        let to := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        let value := StdlibAux.get_calldata_u256 calldata (4 + 32 * 1) in
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 2) calldata) then
          None
        else if negb (get_is_address_valid to) then
          None
        else
          Some (Transfer to value)
      else if selector =? get_selector "allowance(address,address)" then
        let owner := StdlibAux.get_calldata_u256 calldata (4 + 32 * 0) in
        let spender := StdlibAux.get_calldata_u256 calldata (4 + 32 * 1) in
        if negb (callvalue =? 0) then
          None
        else if negb (get_have_enough_calldata (32 * 2) calldata) then
          None
        else if negb (get_is_address_valid owner) then
          None
        else if negb (get_is_address_valid spender) then
          None
        else
          Some (Allowance owner spender)
      else
        None.
End Payload.

Definition main (sender : Address.t) (callvalue : U256.t) (s : Storage.t) (payload : Payload.t) :
    Result.t (Payload.get_return_Set payload * Storage.t) :=
  match payload with
  | Payload.Transfer to value => transfer sender s to value
  | Payload.Approve spender value => approve sender s spender value
  | Payload.TransferFrom from to value => transferFrom sender from to value s
  | Payload.IncreaseAllowance spender addedValue => increaseAllowance sender spender addedValue s
  | Payload.DecreaseAllowance spender subtractedValue =>
    decreaseAllowance sender spender subtractedValue s
  | Payload.TotalSupply => Result.Success (totalSupply s, s)
  | Payload.BalanceOf owner => Result.Success (balanceOf s owner, s)
  | Payload.Allowance owner spender => Result.Success (getAllowance s owner spender, s)
  end.

Parameter keccak256_tuple2 : U256.t -> U256.t -> U256.t.

(** This function describes the internal memory of the smart contract after a successful run. It
    shows some implementation details, that probably depend on how the smart contract is
    compiled. This is not interesting for specifying the smart contract but we still need this
    information explicitly, as we cannot yet hide some specif part of the memory in our
    specification, and an existential is too complex to build. *)
Definition get_memory_end_beginning (sender : Address.t) (payload : Payload.t) : U256.t * U256.t :=
  match payload with
  | Payload.Transfer to _ => (to, 0)
  | Payload.Approve to _ => (to, keccak256_tuple2 sender 1)
  | Payload.TransferFrom from _ _ => (sender, erc20.keccak256_tuple2 from 1)
  | Payload.IncreaseAllowance to _ => (to, erc20.keccak256_tuple2 sender 1)
  | Payload.DecreaseAllowance to _ => (to, erc20.keccak256_tuple2 sender 1)
  | Payload.TotalSupply => (0, 0)
  | Payload.BalanceOf owner => (owner, 0)
  | Payload.Allowance owner spender => (spender, keccak256_tuple2 owner 1)
  end.

Definition body (sender : Address.t) (callvalue : U256.t) (s : Storage.t) (calldata : list Z) :
    Result.t (list U256.t * list U256.t * Storage.t) :=
  match Payload.of_calldata callvalue calldata with
  | Some payload =>
    match main sender callvalue s payload with
    | Result.Success (value, s) =>
      let '(m0, m1) := get_memory_end_beginning sender payload in
      Result.Success (
        [m0; m1],
        Payload.serialize_return payload value,
        s
      )
    | Result.Revert p s => Result.Revert p s
    end
  | None => Result.Revert 0 0
  end.

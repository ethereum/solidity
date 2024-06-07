Require Import CoqOfSolidity.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.

Require Import test.libsolidity.semanticTests.various.erc20.ERC20.

Import Run.

Module Address.
  Definition t : Set :=
    U256.t.
End Address.

Module State.
  Record t := {
    balances : Address.t -> U256.t;
    allowances : Address.t -> Address.t -> U256.t;
    total_supply : U256.t;
  }.

  Definition init : t := {|
    balances := fun _ => 0;
    allowances := fun _ _ => 0;
    total_supply := 0;
  |}.
End State.

Definition constructor (sender : Address.t) : State.t :=
  {|
    State.balances := fun current_address =>
      if current_address =? sender then
        20
      else
        0;
    State.allowances := fun _ _ => 0;
    State.total_supply := 20;
  |}.

Parameter contract_address : Address.t.

Definition get_environment (sender : Address.t) : Environment.t := {|
  Environment.caller := sender;
  Environment.callvalue := 0;
  Environment.calldata := [];
  Environment.address := contract_address;
|}.

Parameter get_storage : forall (state : State.t), Storage.t.

Definition get_state (state : State.t) : CoqOfSolidity.State.t :=
  let account := {|
    Account.balance := 0;
    Account.nonce := 0;
    Account.code := ERC20.deployed.code.(Code.hex_name);
    Account.codedata := [];
    Account.storage := get_storage state;
    Account.immutables := [];
  |} in
  Stdlib.initial_state
    <| State.accounts := [(contract_address, account)] |>
    <| State.codes := ERC20.codes |>.

Lemma run_constructor (sender : Address.t) :
  let environment := get_environment sender in
  let state := get_state State.init in
  {{ environment, state |
    ERC20.code.(Code.code) ⇓ Result.Return ERC20.code.(Code.hex_name) 32
  | get_state (constructor sender) }}.

Definition totalSupply (s : State.t) : U256.t :=
  s.(State.total_supply).

Definition balanceOf (s : State.t) (owner : Address.t) : U256.t :=
  s.(State.balances) owner.

Definition allowance (s : State.t) (owner spender : Address.t) : U256.t :=
  s.(State.allowances) owner spender.

Definition _transfer (from to : Address.t) (value : U256.t) (s : State.t) : option State.t :=
  if to =? 0 then
    None
  else if s.(State.balances) from <? value then
    None
  else
    let s :=
      s <| State.balances := fun addr =>
        if addr =? from then
          (s.(State.balances) from) - value
        else
          s.(State.balances) addr
      |> in
    if s.(State.balances) to + value >=? 2 ^ 256 then
      None
    else
      Some s <|
        State.balances := fun addr =>
          if addr =? to then
            (s.(State.balances) to) + value
          else
            s.(State.balances) addr
      |>.

Definition _mint (account : Address.t) (value : U256.t) (s : State.t) : option State.t :=
  if account =? 0 then
    None
  else
    if s.(State.total_supply) + value >=? 2 ^ 256 then
      None
    else
      let s := s <| State.total_supply := s.(State.total_supply) + value |> in
      if s.(State.balances) account + value >=? 2 ^ 256 then
        None
      else
        Some s <|
        State.balances := fun addr =>
          if addr =? account then
            (s.(State.balances) account) + value
          else
            s.(State.balances) addr
      |>.

Definition _burn (account : Address.t) (value : U256.t) (s : State.t) : option State.t :=
  if account =? 0 then
    None
  else if value >=? s.(State.total_supply) then
    None
  else
    let s := s <| State.total_supply := s.(State.total_supply) - value |> in
    if s.(State.balances) account <? value then
      None
    else
      Some s <|
        State.balances := fun addr =>
          if addr =? account then
            (s.(State.balances) account) - value
          else
            s.(State.balances) addr
      |>.

Definition _approve (owner spender : Address.t) (value : U256.t) (s : State.t) : option State.t :=
  if andb (owner =? 0) (spender =? 0) then
    None
  else
    Some s <|
      State.allowances := fun o sp =>
        if andb (o =? owner) (sp =? spender) then
          value
        else
          s.(State.allowances) o sp
      |>.

Definition _burnFrom (sender : Address.t) (account : Address.t) (value : U256.t) (s : State.t) :
    option State.t :=
  match _burn account value s with
  | None => None
  | Some s =>
    let allowance := s.(State.allowances) account sender in
    if allowance <? value then
      None
    else
      _approve account sender (allowance - value) s
  end.

Definition transfer (sender : U256.t) (s : State.t) (to : Address.t) (value : U256.t) :
    option (bool * State.t) :=
  let s := _transfer sender to value s in
  match s with
  | Some s => Some (true, s)
  | None => None
  end.

Definition approve (sender : U256.t) (s : State.t) (spender : Address.t) (value : U256.t) :
    option (bool * State.t) :=
  let s := _approve sender spender value s in
  match s with
  | Some s => Some (true, s)
  | None => None
  end.

Definition transferFrom (sender : U256.t) (from to : Address.t) (value : U256.t) (s : State.t) :
    option (bool * State.t) :=
  let s := _transfer from to value s in
  match s with
  | None => None
  | Some s =>
    if s.(State.allowances) from sender <? value then
      None
    else
      let s := _approve from sender (s.(State.allowances) from sender - value) s in
      match s with
      | None => None
      | Some s => Some (true, s)
      end
  end.

Definition increaseAllowance (sender : U256.t) (spender : Address.t) (addedValue : U256.t)
    (s : State.t) : option (bool * State.t) :=
  if s.(State.allowances) sender spender + addedValue >=? 2 ^ 256 then
    None
  else
    let s := _approve sender spender (s.(State.allowances) sender spender + addedValue) s in
    match s with
    | Some s => Some (true, s)
    | None => None
    end.

Definition decreaseAllowance (sender : U256.t) (spender : Address.t) (subtractedValue : U256.t)
    (s : State.t) : option (bool * State.t) :=
  if s.(State.allowances) sender spender <? subtractedValue then
    None
  else
    let s := _approve sender spender (s.(State.allowances) sender spender - subtractedValue) s in
    match s with
    | Some s => Some (true, s)
    | None => None
    end.

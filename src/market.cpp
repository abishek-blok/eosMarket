#include <market.hpp>

ACTION market::hi(name from, string message) {
  require_auth(from);

  // Init the _message table
  messages_table _messages(get_self(), get_self().value);

  // Find the record from _messages table
  auto msg_itr = _messages.find(from.value);
  if (msg_itr == _messages.end()) {
    // Create a message record if it does not exist
    _messages.emplace(from, [&](auto& msg) {
      msg.user = from;
      msg.text = message;
    });
  } else {
    // Modify a message record if it exists
    _messages.modify(msg_itr, from, [&](auto& msg) {
      msg.text = message;
    });
  }
}

ACTION market::clear() {
  require_auth(get_self());

  messages_table _messages(get_self(), get_self().value);

  // Delete all records in _messages table
  auto msg_itr = _messages.begin();
  while (msg_itr != _messages.end()) {
    msg_itr = _messages.erase(msg_itr);
  }
}

ACTION market::listcard(name from, uint64_t asset_id,asset price){
require_auth(from);
 list_table _list(get_self(), get_self().value);
 auto list_itr = _list.find(asset_id);
  if (list_itr == _list.end()) {
  _list.emplace(from,[&](auto &list)
                                 {
                                     list.asset_id = asset_id;
                                     list.owner = from;
                                     list.price = price;
                                     list.time = time_point_sec(current_time_point());
                                     list.status = "listed";
                                 });
  }else{
    _list.modify(list_itr,from, [&](auto &list)
                                 {
                                     list.price = price;
                                 });
}
}

ACTION market::addbalance(name from, asset amount){
require_auth(from);

   token_table _token(get_self(), get_self().value);
    check(_token.begin()!=_token.end(), "token not added yet");
    auto token = _token.begin();
    check(amount.symbol.code().raw() == token->sys_val.code().raw(),"this token is not accepted");

 balance_table _balance(get_self(), get_self().value);
 auto balance_itr = _balance.find(from.value);
  if (balance_itr == _balance.end()) {
  _balance.emplace(from,[&](auto &balance)
                                 {
                                     balance.account = from;
                                     balance.available_balance = amount;
                                     balance.locked_balance.amount = 0;
                                 });
  }else{
    _balance.modify(balance_itr,from, [&](auto &balance)
                                 {
                                     balance.available_balance += amount;
                                 });
}
}

  ACTION market::buy(name from, uint64_t asset_id){
    require_auth(from);
    list_table _list(get_self(), get_self().value);
    auto list_itr = _list.find(asset_id);
      if (list_itr == _list.end()) {
      print("asset not listed ");
      }else{
      check (list_itr->status == "listed","card not listed any more");
      balance_table _balance(get_self(), get_self().value);
      auto balance_itr = _balance.find(from.value);
      if (balance_itr == _balance.end()) {
        print("balance  not added ");
      }else{
        check(balance_itr->available_balance.amount >= list_itr->price.amount, "user dont have enough balance");
         _balance.modify(balance_itr,from, [&](auto &balance)
                                 {
                                     balance.available_balance -= list_itr->price;
                                 });
             _list.modify(list_itr,from,[&](auto &list)
                              {
                              list.status = "sold";
                              });   
        send_summary(from, " Card bought successfully");                 
      /// update seller balance
        auto balance_seller_itr = _balance.find(list_itr->owner.value);
      if (balance_seller_itr == _balance.end()) {
        print("asset not listed ");
          _balance.emplace(from,[&](auto &balance)
                                 {
                                 //1 is added just emulate marketplace fee
                                     balance.account = list_itr->owner;
                                     balance.available_balance.amount = list_itr->price.amount - 1;
                                     balance.locked_balance.amount = 0 ;
                                 });
        }else{
         _balance.modify(balance_seller_itr,from, [&](auto &balance)
                                 {
                                 //1 is added just emulate marketplace fee
                                     balance.available_balance.amount += list_itr->price.amount  - 1;
                                 });
        }
      ///
      }
    }
   }


ACTION market::addtoken(symbol sys){
require_auth(get_self());
 token_table _token(get_self(), get_self().value);
 check(_token.begin()==_token.end(), "token already exist");
 
  _token.emplace(get_self(),[&](auto &token)
                                 {
                                     token.sys_val = sys;
                                 });
  }

 ACTION market::notify(name user, string msg) {
    require_auth(get_self());
    require_recipient(user);
  }


  void market::send_summary(name user, std::string message) {
      action(
        permission_level{get_self(),"active"_n},
        get_self(),
        "notify"_n,
        std::make_tuple(user, name{user}.to_string() + message)
      ).send();
    }

    void market::transfer(){
    //just for testing paybale action
      action{
        permission_level{get_self(), "active"_n},
        "eosio.token"_n,
        "transfer"_n,
        std::make_tuple(get_self(), "abishek12345", "10 wax", string("teestt"))
      }.send();
    }



    ACTION market::addoffer(name from, uint64_t asset_id,asset price){
    require_auth(from);
    list_table _list(get_self(), get_self().value);
    auto list_itr = _list.find(asset_id);
      if (list_itr == _list.end()) {
          print("asset not listed ");
        }else{
          check (list_itr->status == "listed","card not listed any more");
          balance_table _balance(get_self(), get_self().value);
          auto balance_itr = _balance.find(from.value);
          if (balance_itr == _balance.end()) {
            print("balance  not added ");
          }else{
          check(balance_itr->locked_balance.amount < list_itr->price.amount, "offer price connot be more or equal to actual price");


  offer_table _offer(get_self(), get_self().value);
      auto offer_itr = _offer.find(asset_id);
      if (offer_itr == _offer.end()) {
        _balance.modify(balance_itr,from, [&](auto &balance)
                                 {
                                     balance.available_balance -= price;
                                     balance.locked_balance.amount = price.amount;
                                 });

        _offer.emplace(from,[&](auto &offer)
                                 {
                                     offer.asset_id = asset_id;
                                     offer.price = price;
                                     offer.offeredBy = from ;
                                 });                         
      }else{
       print("offer already added ");
      }
      }}
   }











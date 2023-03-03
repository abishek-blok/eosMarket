#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

using namespace std;
using namespace eosio;

CONTRACT market : public contract {
  public:
    using contract::contract;

    ACTION hi(name from, string message);
    ACTION clear();
    ACTION testa(name from);
    ACTION listcard(name from, uint64_t asset_id,asset price);
    //ACTION addbalance(name from,asset amount);
    ACTION buy(name from,uint64_t asset_id);
    ACTION addtoken(symbol sys);
    ACTION notify(name user, string msg);
    ACTION addoffer(name from, uint64_t asset_id,asset price);
    //[[eosio::on_notify("eosio.token::transfer")]] ACTION deposit(name from,name to, uint64_t asset_id,asset price);
    [[eosio::on_notify("eosio.token::transfer")]] void deposit(name from, name to, eosio::asset quantity, std::string memo);

    ACTION multitest(asset price);
 
  private:
    TABLE messages {
      name    user;
      string  text;
      auto primary_key() const { return user.value; };
    };
    typedef multi_index<name("messages"), messages> messages_table;

    private:
    TABLE token_struct
    {
        symbol sys_val;
        auto primary_key() const { return sys_val.code().raw(); };
    };

    //typedef multi_index<name("token"), token_struct> token_table;
    //typedef multi_index<name("token"), token_struct> token_table;
    //typedef singleton<name("token"), token_struct> token_table;
    typedef singleton<"token"_n, token_struct> token_table;
    typedef eosio::multi_index<"token"_n, token_struct> token_table_for_abi;
    //token_table _token;


    TABLE list {
     uint64_t asset_id;
     name owner;
     asset price;
     time_point_sec  time;
    string status;

    auto primary_key() const {return asset_id;}
    uint64_t by_price() const {return price.symbol.code().raw();}
    } ;
      typedef multi_index<name("list"), list,
      indexed_by<"price"_n,const_mem_fun<list,uint64_t,&list::by_price>>> list_table;


    TABLE balance_struct
    {
        name account;
        asset available_balance;
        asset locked_balance;
        auto primary_key() const { return account.value; };  
    };
    typedef multi_index<name("balance"), balance_struct> balance_table;
   // balance_table _balance;


    TABLE offer_struct
    {
        uint64_t asset_id;
        asset price;
        name offeredBy;
        auto primary_key() const { return asset_id; }  ;
    };
    typedef multi_index<name("offers"), offer_struct> offer_table;
   // offer_table _offer;

void send_summary(name user, string message);
void addbalance(name from, asset amount);
void transfer();
};

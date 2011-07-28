/*
 *  leveldb.h
 *  leveldb
 *
 *  Created by Toshiro Yamada on 7/27/11.
 *  Copyright (C) 2011 UCSD. All rights reserved.
 */
#ifndef __MAX_LEVELDB__
#define __MAX_LEVELDB__

#include <cstring>
#include <string>
#include <tr1/unordered_map>

extern "C" {
#include "ext.h"
#include "ext_obex.h"
}

#include <leveldb/db.h>
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<leveldb::DB> LevelDBPtr;

struct eqstr {
  bool operator()(const char* lhs, const char* rhs) const {
    return strcmp(lhs, rhs) == 0;
  }
};

class MaxClass
{
public:
  MaxClass() {}
  ~MaxClass() {}
  
  //! Called when Max creates the object
  static void* alloc(t_symbol* s, long argc, t_atom* argv);
  
  //! Called when object is deleted
  static void free(MaxClass* x);
  
  //! Inlet and outlet assistant messages
  static void assist(MaxClass* x, void* b, long m, long a, char* s);

  static void put(MaxClass* x, t_symbol* key, t_symbol* value);
  static void get(MaxClass* x, t_symbol* key);
  static void remove(MaxClass* x, t_symbol* key);
  static void open(MaxClass* x, t_symbol* name);
  static void close(MaxClass* x);
  
  bool open_db(const char* db_name);
  
	t_object object_;
  void* outlet_;
  
  // store a list of databases in a hashtable
  static std::tr1::unordered_map<const char*, LevelDBPtr, std::tr1::hash<const char*>, eqstr> db_list;
  
private:  
  bool check_db();
  bool check_db_error(const leveldb::Status& status);
  bool check_db_error(const leveldb::Status& status, const char* key);
  
  const char* db_name_;
  LevelDBPtr db_;
};

#endif // __MAX_LEVELDB__

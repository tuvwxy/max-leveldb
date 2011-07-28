/*
 *  MaxClass.cpp
 *  MaxClass
 *
 *  Created by Toshiro Yamada on 7/27/11.
 *  Copyright (C) 2011 UCSD. All rights reserved.
 */
#include "MaxClass.h"

// Global class pointer
static t_class* s_leveldb_class = NULL;
std::tr1::unordered_map<const char*, LevelDBPtr, std::tr1::hash<const char*>, eqstr> MaxClass::db_list;

#pragma mark -
#pragma mark Alloc & Free
void* MaxClass::alloc(t_symbol* s, long argc, t_atom* argv)
{
  if (argc != 1) {
    post("leveldb: must specify a name");
    return NULL;
  } else if (atom_gettype(argv) != A_SYM) {
    post("leveldb: name must be a symbol");
    return NULL;
  }
  
	MaxClass* x = (MaxClass*) object_alloc(s_leveldb_class);
	if (!x) {
    return NULL;
  }
  
  const char* db_name = atom_getsym(argv)->s_name;
  if (!x->open_db(db_name)) {
    object_free(x);
    return NULL;
  }
  
  x->outlet_ = outlet_new(x, NULL);
	return x;
}

void MaxClass::free(MaxClass* x) 
{
  delete x;
}

bool MaxClass::open_db(const char* db_name)
{
  if (db_list.find(db_name) != db_list.end()) {
    db_ = db_list[db_name];
  } else {
    leveldb::Options options;
    options.create_if_missing = true;
    
    std::string db_path;
    if (db_name[0] == '/') {
      db_path = db_name;
    } else {
      db_path = "/tmp/max_leveldb_";
      db_path.append(db_name);
    }
    
    leveldb::DB* db;
    leveldb::Status status = leveldb::DB::Open(options, db_path, &db);
    if (!check_db_error(status)) {
      return false;
    }
    
    object_post(&object_, "new db at %s", db_path.c_str());
    db_ = LevelDBPtr(db);
    MaxClass::db_list[db_name] = db_;
  }
  db_name_ = db_name;
  return true;
}

bool MaxClass::check_db()
{
  if (!db_.get()) {
    object_error(&object_, "database is not opened");
    return false;
  }
  return true;
}

bool MaxClass::check_db_error(const leveldb::Status& status)
{
  if (!status.ok()) {
    object_error(&object_, "%s", status.ToString().c_str());
    return false;
  }
  return true;
}

bool MaxClass::check_db_error(const leveldb::Status& status, const char* key)
{
  if (!status.ok()) {
    object_error(&object_, "%s%s", status.ToString().c_str(), key);
    return false;
  }
  return true;
}

void MaxClass::put(MaxClass* x, t_symbol* key, t_symbol* value)
{
  if (x->check_db() && key && value) {
    if (!strcmp(key->s_name, "")) {
      object_post(&x->object_, "put: key is empty");
      return;
    } else if (!strcmp(value->s_name, "")) {
      object_post(&x->object_, "put: value is empty");
      return;
    }
    leveldb::WriteOptions options;
    options.sync = true;
    leveldb::Status status = x->db_->Put(options, key->s_name, value->s_name);
    x->check_db_error(status, key->s_name);
  }
}

void MaxClass::get(MaxClass* x, t_symbol* key)
{
  if (x->check_db() && key) {
    if (!strcmp(key->s_name, "")) {
      object_post(&x->object_, "get: key is empty");
      return;
    }
    
    std::string value;
    leveldb::Status status = 
        x->db_->Get(leveldb::ReadOptions(), key->s_name, &value);
    if (x->check_db_error(status, key->s_name)) {
      outlet_anything(x->outlet_, gensym(value.c_str()), 0, NULL);
    }
  }
}

void MaxClass::remove(MaxClass* x, t_symbol* key)
{
  if (x->check_db() && key) {
    if (!strcmp(key->s_name, "")) {
      object_post(&x->object_, "delete: key is empty");
      return;
    }
    
    leveldb::Status status = 
        x->db_->Delete(leveldb::WriteOptions(), key->s_name);
    x->check_db_error(status, key->s_name);
  }
}

void open(MaxClass* x, t_symbol* name)
{
  if (!name || !name->s_name) {
    object_error(&x->object_, "open: no argument");
    return;
  }
  
  if (!x->open_db(name->s_name)) {
    object_error(&x->object_, "open: could not open database %s", name->s_name);
  }
}

void MaxClass::close(MaxClass* x)
{
  x->db_ = LevelDBPtr();
}


#pragma mark -
#pragma mark Max Helpers
void MaxClass::assist(MaxClass* x, void* b, long m, long a, char* s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
			case 0:
				sprintf(s, "1st Inlet");
				break;
		}
	} 
	else {
		switch (a) {
			case 0:
				sprintf(s, "1st Outlet");
				break;
		}		
	}
}

#pragma mark -
#pragma mark Main
extern "C" int main(void)
{
	t_class* c;
	
	c = class_new("leveldb", (method)MaxClass::alloc, (method)MaxClass::free,
      sizeof(MaxClass), (method)0L, A_GIMME, 0);
	
	/* Add class methods */
	class_addmethod(c, (method)MaxClass::assist, "assist", A_CANT, 0);
  class_addmethod(c, (method)MaxClass::put, "put", A_SYM, A_SYM, 0);
  class_addmethod(c, (method)MaxClass::get, "get", A_SYM, 0);
  class_addmethod(c, (method)MaxClass::remove, "delete", A_SYM, 0);
  class_addmethod(c, (method)MaxClass::close, "close", A_SYM, 0);
  class_addmethod(c, (method)MaxClass::close, "close", 0);
	
	/* Add attributes */
	
	class_register(CLASS_BOX, c);
	s_leveldb_class = c;
	post("leveldb 0.1 by Toshiro Yamada. "
       "Copyright (c) 2011 The Regents of University of California");
	return 0;
}

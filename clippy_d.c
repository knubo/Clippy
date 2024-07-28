#include <mudlib.h>

inherit I_DAEMON;
inherit I_STRINGTOOLS;

mapping wizards;

static void create() {
  ::create();

  wizards = ([]);
}

void register() {
  wizards[previous_object()] = environment(previous_object());
}

void unregister() {
  map_delete(wizards, previous_object());
}

void present_errors(int when, string path) {
  string err, test, target;
  string *lines;
  int i, sz, said_something, read_length;
  object who;


  if(!when) {
    write("Must be called with a when argument.\n");
    return;
  }
  
  if(!wizards[previous_object()]) {
    wizards[previous_object()] = environment(previous_object());
  }

  who = wizards[previous_object()];

  if(path[0..2] == "/d/") {
    target = explode(path, "/")[1];
    path = "/d/"+target;
  } else if(strlen(path) > 9 && path[0..8] == "/players/") {
    target = explode(path, "/")[1];
    path = "/players/"+target;
  } else {
    path = "";
  }

  if(target) {
    call_out("check_for_runtime_errors", 1, who, path, when, target);
    call_out("check_for_runtime_errors", 5, who, path, when+4, target);
    call_out("check_for_runtime_errors", 10, who, path, when+9, target);
  }
  
  read_length = file_size(path+"/log/compile.err");

  if(read_length < 1) {
    return;
  }
  
  if(read_length > 80*20) {
    read_length = 80*20;
  }

  
  err = read_bytes(path+"/log/compile.err", -read_length, read_length);

  lines = explode(err, "\n");

  if(!sizeof(lines)) {
    return;
  }
  
  test = ctime(when)[4..];
  
  for(i=0, sz = sizeof(lines);i < sz; i++) {
    
    if(strlen(lines[i]) >= strlen(test) && lines[i][0..11] == test[0..11]) {
      
      if(lines[i][0..strlen(test)-1] >= test) {
	if(!said_something) {
	  tell_object(who, "%^L_RED%^Clippy says:%^END%^\n");
	  said_something = 1;
	}
	
	tell_object(who, lines[i]+"\n");
      }
    }
  }
}

void look_for_runtime_errors(object who, string *lines, string target, string path) {
  string txt;

  if(strsrch(lines[0], "Failed to compile") > 0 || strsrch(lines[0], "Could not compile") > 0) {
    return;
  }
  
  lines[0] = lines[0][0..20]+"%^BOLD%^"+lines[0][21..]+"%^END%^";
  
  txt = implode(lines, "\n");

  if(strsrch(txt, target) == -1) {
    return;
  }

  tell_object(who, "%^L_RED%^Clippy says: I found some runtime errors:%^END%^ - in "+path+"/log/runtime.err\n"+
	      replace_string(txt, target, "%^L_YELLOW%^"+target+"%^END%^")+"\n");
  
}

void check_for_runtime_errors(object who, string path, int when, string target) {
  string *lines;
  int i, sz, said_something, read_length, found;
  string err, test;

  if(!who) {
    return;
  }
 
  read_length = file_size(path+"/log/runtime.err");

  if(read_length < 1) {
    if(path != "") {
      call_out("check_for_runtime_errors", 0, who, "", when, target);
    }
    return;
  }

  
  if(read_length > 80*20) {
    read_length = 80*20;
  }
  
  err = read_bytes(path+"/log/runtime.err", -read_length, read_length);
  test = ctime(when)[4..];

  lines = explode(err, "\n");

  for(i=0, sz = sizeof(lines);i < sz; i++) {

    if(strlen(lines[i]) >= strlen(test) && lines[i][0..11] == test[0..11]) {

      if(lines[i][0..strlen(test)-1] >= test) {
	if(found) {
	  look_for_runtime_errors(who, lines[found..(i-1)], target, path);
	}
	found = i;
      }
    }
  }

  if(found && found < i) {
    look_for_runtime_errors(who, lines[found..(i-1)], target, path);
  }
  
  if(!found && strlen(path)) {
    call_out("check_for_runtime_errors", 0, who, "", when, target);
  }
  
}



void debug() {
  printf("%O\n", wizards);
}

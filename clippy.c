#include <mudlib.h>
#include <std.h>
#include <levels.h>

inherit item I_ITEM;
inherit analyze "/players/knubo/clippy/analyze";

static void create() {
  item::create();
  analyze::create();

  set_name("clippy");

  set_short("clippy");

  set_long("Clippy, the happy programming helper. It activates when your current directory is different than your home directory. After a successful refresh, it will start tracking that file. After the timestamp of that file changes, it will automatically refresh that file, and whatever errors happens Clippy will print out to you. In addition it does:\n\n * If it detects an inheritance hierarchy, with exact file name, like inherit \"/d/Golem/lib/guildlevel\";, it will refresh things with this inherit if guildlevel.c changes.\n * If an item in your inventory is refreshed, it will destroy that clone and create a new one.\n\nIf there is no change within a little period of time, Clippy will stop scanning the files you have freshed. Just fresh any file, and it will resume. \n\nIf you want to clear what files clippy is tracking, use \"track clear\".\n\nIf you want to auto run tests upon an object refreshed, add a function string meta_test() which should return file name of the test.");

  set_drop(NO_DROP);

  if(clonep(this_object())) {
    call_out("check_env", 2);
  }

  add_trigger("load", store_fp("refresh_done"));
  add_trigger("freshall", store_fp("refresh_done"));
  add_trigger("fresh", store_fp("refresh_done"));
  add_trigger("refresh", store_fp("refresh_done"));
  add_trigger("clone", store_fp("refresh_done"));
  add_trigger("?", store_fp("refresh_done"));
  add_trigger("exec", store_fp("refresh_done"));
  add_trigger("track", store_fp("add_track"));
  
  add_trigger("patch", store_fp("do_patch"));
  add_property("tagged");
}


int refresh_done(string arg) {
  object ob;
  if(!arg) {
    arg = ".";
  }

  if(query_verb() == "refresh") {
    ob = "/com/w/fresh"->query_my_last_object();
  }
  
  if(!ob) {
    ob = present(arg, this_player());
  }
  if(!ob) {
    ob = present(arg, environment(this_player()));
  }

  analyze(arg);
  
  if(ob) {
    call_out("check_for_error", 1, time(), file_name(ob));
    call_out("check_consistency", 2, arg, file_name(environment(this_player())));        
  } else {
    call_out("check_for_error", 1, time(), this_player()->resolve_path(arg));
    call_out("check_consistency", 2, arg, file_name(environment(this_player())));    
  }
  return 0;
}

int do_patch(string arg) {
  string *parts;
  object ob;

  if(!arg) {
    return 0;
  }
  
  parts = explode(arg, " ");

  if((ob = present(parts[0], environment(this_player()))) || (ob = present(parts[0], this_player()))) {
    call_out("check_for_error", 1, time(), source_file_name(ob));
  } else {  
    call_out("check_for_error", 1, time(), this_player()->resolve_path(parts[0]));
  }
  return 0;
  
}

void check_for_error(int w, string path) {
  call_other("/players/knubo/clippy/clippy_d","present_errors", w, path);
}


void check_env() {
  if(!living(environment(this_object()))) {
    move(R_VOID);
    return;
  }

  if(environment(this_object())->query_level() < STUDENT) {
    write("You don't need this ya know.\n");
    move(R_VOID);
    return;
  }

  tell_object(environment(this_object()), "Clippy is at your service.\n");

  "/players/knubo/clippy/clippy_d"->register();
}

void on_destruct() {
  ::on_destruct();

  "/players/knubo/clippy/clippy_d"->unregister();
}
  
string query_auto_load() {
  return "/players/knubo/clippy/clippy:";
}

void init_arg(string arg) {

}

void check_consistency(string arg, string env) {
  object thing;
  int c;
      
  if(!this_player()) {
    return;
  }

  thing = find_object(env);
  
  if(!thing) {
    return;
  }
  
  if(arg != "env") {
    thing = find_object(arg);

    if(!thing) {
      thing = find_object(this_player()->resolve_path(arg));
    }
  }  
  
  if(!thing) {
    return;
  }

  if(thing <- I_ITEM) {
    if(thing->short() && !thing->query_name()) {
      write("%^L_CYAN%^Clippy hints: Your item, "+file_name(thing)+", is missing name. You probably want to add one.%^END%^\n");
    }
  }
  
  if(thing <- I_ROOM) {
    if(!thing->short()) {
      write("%^L_CYAN%^Clippy hints: Your room, "+file_name(thing)+", is missing short. You probably want to add one.%^END%^\n");
    } else {

      c = thing->short()[0];
      
      if(c >= 'a' && c <= 'z') {
	write("%^L_CYAN%^Clippy hints: Your short, "+thing->short()+", will look nicer if it has uppercase letter.%^END%^\n");	
      }
    }
    
  }

}

void show_stats() {
  printf("Clippy info so far:\n");
  
  analyze::show_stats();
}

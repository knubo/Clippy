#include <mudlib.h>

mapping timestamps;
mapping dependencies;

string *track;

void find_dependencies(string file);

void create() {
  timestamps = ([]);
  dependencies = ([]);
  track = ({});
}

int add_track(string t) {

  if(t == "clear") {
    write("Clearing tracks.\n");
    track = ({});
    return 1;
  }
  
  track |= ({t});
  printf("Starting to track %s.\n", t);

  return 1;
}

void file_modified(string file, int when, object tp) {
  timestamps[file] = when;
  
  tell_object(tp, sprintf("File %s was last modified at %s.\n", file, ctime(when)));
}

int idle_path(object tp) {
  return tp->query_path() == ("/players/"+tp->query_real_name());
}

void analyze(string arg) {
  object tp;
  string file;
  mixed *dirinfo;
  object ob;
  
  tp = this_player();

  if(!arg) {
    return;
  }
  
  ob = xfind_object(arg);

  if(ob) {
    arg = source_file_name(ob);
  }
  
  if(!sscanf(arg, "%*s.c")) {
    arg = arg+".c";
  }

  file = tp->resolve_path(arg);
  
  if(file[0..2] != "/d/" && file[0..8] != "/players/" && !sizeof(({file}) & track)) {

    if(ob && ob <- I_TEST) {
      write("Allowing the test through.\n");
    } else {
      printf("Clippy analyze ignoring %s.\n", arg);
      return;
    }
  }

  call_out("run_tests", 0, file);
  
  dirinfo = get_dir(file);
  
  if(!dirinfo || sizeof(dirinfo[2]) != 1) {
    return;
  }

  find_dependencies(file);
  
  if(!timestamps[file]) {
    timestamps[file] = dirinfo[2][0];
  }

  if(timestamps[file] && timestamps[file] != dirinfo[2][0]) {
    file_modified(file, dirinfo[2][0], tp);    
  }
  
  if(find_call_out("track_changes") == -1 && !idle_path(tp)) {
    call_out("track_changes", 4, tp);
    printf("Starting tracking changes.\n");
  }
}

void run_tests(string file) {
  object ob;

  if(file_size(file) <= 0) return;
  ob = load_object(file);
  if(ob && ob->meta_test()) {
    object test;

    test = load_object(ob->meta_test());
    test->run();
  }

}

void refresh_objects_in_inventory(object tp, object ob) {
  object *inv;
  int i;
  string filename;

  filename = file_name(ob);
  
  inv = all_inventory(tp);

  for(i=sizeof(inv);i--;) {
    if(file_name(inv[i]) == filename) {
      object n;
      string autoload_info;
      
      n = clone_object(filename);

      printf("Replacing %O with %O in your inventory.\n", inv[i], n);

      /* Fetches auto load info and transfers it to next object */
      autoload_info = inv[i]->query_auto_load();
      if(autoload_info) {
	string info;
	
	if(sscanf(autoload_info, "%*s:%s", info) == 2) {
	  n->init_arg(info);
	}
      }
      
      inv[i]->destroy();
      n->move(tp);

    }
  }
}

void reload_file(string file, object tp) {
  string obj;
  string *k;
  int i;
  object ob;

  if(!tp) {
    return;
  }
  
  tp->command("fresh "+file);
  
  ob = find_object(file);

  if(ob) {
    refresh_objects_in_inventory(tp, ob);
  }
  
  if(ob && ob <- I_TEST) {
    ob->run();
  }

  if(ob && ob->meta_test()) {
    object test;

    test = load_object(ob->meta_test());
    test->run();
  }
  
  if(!sscanf(file, "%s.c", obj)) {
    tell_object(tp, "I got it wrong with "+file+".\n");
    return;
  }
  
  k = keys(dependencies);

  for(i=sizeof(k);i--;) {

    if(sizeof( dependencies[k[i]] & ({obj}) ) ) {
      reload_file(k[i], tp);
    }
  }
}

void track_changes(object tp) {
  int i;
  string *k;

  if(!tp) {
    write("I got no TP?\n");
    return;
  }

  k = keys(timestamps);
  for(i=sizeof(k);i--;) {
    mixed *dirinfo;
    string file;

    file = k[i];

    dirinfo = get_dir(file);
    
    if(!dirinfo || !sizeof(dirinfo)) {
      map_delete(timestamps, file);
      continue;
    }

    if(sizeof(dirinfo) < 3 || !dirinfo[2] || !sizeof(dirinfo[2])) {
      continue;
    }
    
    if(timestamps[file] < dirinfo[2][0]) {
      file_modified(file, dirinfo[2][0], tp);
      reload_file(file, tp);
    }
    
  }

  if(idle_path(tp) || !query_ip_number(tp) || query_idle(tp) > 900) {
    tell_object(tp, "Pausing auto refresh.\n");
  } else {
    call_out("track_changes", 4, tp);
  }
  
}

void debug() {
  printf("%O\n%O\n", timestamps, dependencies);
}

void find_dependencies(string file) {
  string *lines;
  int i, sz;
  
  lines = read_lines(file);

  if(!lines) {
    return;
  }
  for(i=0,sz = sizeof(lines); i < sz; i++) {
    string dep;
    
    if(sscanf(lines[i], "%*sinherit%*s\"%s\";", dep) && dep) {
      analyze(dep);
      if(!dependencies[file]) {
	dependencies[file] = ({dep});
      } else {
	dependencies[file] = dependencies[file] | ({dep});
      }
    }
  }
}

void show_stats() {
  int i;
  string *k;

  k = keys(timestamps);

  for(i=sizeof(k);i--;) {
    printf("File: %s touched %s.\n", k[i], ctime(timestamps[k[i]]));
  }

  k = keys(dependencies);

  for(i=sizeof(k);i--;) {
    printf("File %s deps: %O\n", k[i], dependencies[k[i]]);
  }
}
